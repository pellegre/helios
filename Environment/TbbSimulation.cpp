/*
Copyright (c) 2012, Esteban Pellegrino
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <omp.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include "Simulation.hpp"

using namespace std;

namespace Helios {

namespace IntelTbb {

class SourceSimulator {
	/* Base random number stream */
	const Random& base;
	/* Reference to particle container */
	vector<Simulation::CellParticle>& particles;
	/* Upper bound random numbers on source generation */
	size_t max_rng;

	/* Stuff got from the environment */
	const Source* source;       /* Source defined on the problem */
	const Geometry* geometry;   /* Geometry of the problem */
public:

	SourceSimulator(const McEnvironment* environment, const Random& base, vector<Simulation::CellParticle>& particles,
			const size_t& max_rng_per_source) : base(base), particles(particles), max_rng(max_rng),
			source(environment->getModule<Source>()), geometry(environment->getModule<Geometry>()) {/* */}

	void operator() (const tbb::blocked_range<size_t>& range) const {
		for(size_t i = range.begin() ; i < range.end() ; ++i) {
			Random r_local(base);
			r_local.getEngine().jump(i * max_rng);
			/* Sample particle */
			Particle particle = source->sample(r_local);
			const Cell* cell(geometry->findCell(particle.pos()));
			particles[i] = Simulation::CellParticle(cell->getInternalId(),particle);
		}
	}

	virtual ~SourceSimulator() {/* */}
};

class PowerStepSimulator {
	/* Base random number stream */
	const Random& base;
	/* Upper bound random numbers on source generation */
	size_t max_rng;
	/* Current particle bank (source of last step) */
	vector<Simulation::CellParticle>& current_bank;
	/* Particle local bank container (save particles states after the simulation) */
	vector<Simulation::CellParticle>& after_bank;
	/* Stuff got from the environment */
	const Geometry* geometry;   /* Geometry of the problem */
public:
	/* Population after the simulation */
	double local_population;

	PowerStepSimulator(const McEnvironment* environment, const Random& base, const size_t& max_rng,
			vector<Simulation::CellParticle>& current_bank,vector<Simulation::CellParticle>& after_bank) :
			base(base), max_rng(max_rng), current_bank(current_bank),
			after_bank(after_bank), geometry(environment->getModule<Geometry>()), local_population(0.0)
			{/* */}

	PowerStepSimulator(PowerStepSimulator& right, tbb::split) : base(right.base), max_rng(right.max_rng),
			current_bank(right.current_bank), after_bank(right.after_bank), geometry(right.geometry),
			local_population(0.0) {/* */}

	void join(PowerStepSimulator& right) {
		local_population += right.local_population;
	}

	void operator() (const tbb::blocked_range<size_t>& range) {

		/* Get population */
		double population = local_population;

		/* Local bank */
		vector<Simulation::CellParticle>& local_fission_bank = after_bank;

		/* Bank to be simulated */
		vector<Simulation::CellParticle>& fission_bank = current_bank;

		/* Geometry stuff */
		Surface* surface(0);  /* Surface pointer */
		bool sense(true);     /* Sense of the surface we are crossing */
		double distance(0.0); /* Distance to closest surface */

		for(size_t i = range.begin() ; i < range.end() ; ++i) {

			Random r_local(base);
			r_local.getEngine().jump(i * max_rng);

			/* Get particle from the user supplied bank */
			Simulation::CellParticle pc = fission_bank[i];
			const Cell* cell = geometry->getCells()[pc.first];
			Particle particle = pc.second;

			/* Flag if particle is out of the system */
			bool out = false;

			while(true) {

				/* Get next surface and distance */
				cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

				/* Energy index of the particle */
				EnergyIndex energy_index = particle.eix();
				/* Get material */
				const Material* material = cell->getMaterial();
				/* Get total cross section */
				double mfp = material->getMeanFreePath(energy_index);

				/* Get collision distance */
				double collision_distance = -log(r_local.uniform())*mfp;

				while(collision_distance > distance) {
					/* Cut on a vacuum surface */
					if(surface->getFlags() & Surface::VACUUM) {
						out = true;
						break;
					}

					/* Transport the particle to the surface */
					particle.pos() = particle.pos() + distance * particle.dir();

					if(surface->getFlags() & Surface::REFLECTING) {
						/* Get normal */
						Direction normal;
						surface->normal(particle.pos(),normal);
						/* Reverse if necessary */
						if(sense == false) normal = -normal;
						/* Calculate the new direction */
						double projection = 2 * dot(particle.dir(), normal);
						particle.dir() = particle.dir() - projection * normal;
					} else {
						/* Now get next cell */
						surface->cross(particle.pos(),sense,cell);
						if(!cell) {
							cout << particle << endl;
							cout << *surface << endl;
						}
						/* Cut if the cell is dead */
						if(cell->getFlag() & Cell::DEADCELL) {
							out = true;
							break;
						}
					}

					/* Calculate new distance to the closest surface */
					cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

					/* Update material */
					material = cell->getMaterial();
					mfp = material->getMeanFreePath(energy_index);

					/* And the new collision distance */
					collision_distance = -log(r_local.uniform())*mfp;
				}

				if(out) break;

				/* If we are out, we reach some point inside a cell were a collision should occur */
				particle.pos() = particle.pos() + collision_distance * particle.dir();

				/* get and apply reaction to the particle */
				Reaction* reaction = material->getReaction(particle.eix(),r_local);
				(*reaction)(particle,r_local);

				if(particle.sta() == Particle::DEAD) break;
				if(particle.sta() == Particle::BANK) {
					/* Update local particle bank */
					population += particle.wgt();
					local_fission_bank[i] = Simulation::CellParticle(cell->getInternalId(),particle);
					break;
				}
			}
		}

		/* Save new population */
		local_population = population;

	}

	virtual ~PowerStepSimulator() {/* */}
};

KeffSimulation::KeffSimulation(const Random& _random, McEnvironment* _environment, double keff, size_t _particles_number) :
		Simulation(_random,_environment), keff(keff), particles_number(_particles_number) {

	/* Get geometry from the environment */
	geometry = environment->getModule<Geometry>();

	/* Reserve space for the particle bank */
	fission_bank.reserve(2 * particles_number);
	fission_bank.resize(particles_number);

	/* Populate the particle bank with the initial source */
	tbb::parallel_for(tbb::blocked_range<size_t>(0,particles_number),SourceSimulator(environment,base,fission_bank,max_rng_per_source));

	/* Jump on base stream of RNGs */
	base.getEngine().jump(particles_number * max_rng_per_source);
}

void KeffSimulation::launch() {

	/* --- Local particle bank for for this simulation */
	vector<CellParticle> local_fission_bank(fission_bank);

	PowerStepSimulator power_step(environment,base,max_rng_per_history,fission_bank,local_fission_bank);

	/* Simulate power step */
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, fission_bank.size()), power_step);

	/* Jump on random number generation */
	base.getEngine().jump(fission_bank.size() * max_rng_per_history);

	/* --- Calculate multiplication factor for this cycle */
	keff = power_step.local_population / (double) particles_number;

	/* --- Clear particle bank*/
	fission_bank.clear();

	/* --- Re-populate the particle bank with the new source */
	while(!local_fission_bank.empty()) {
		/* Get banked particle */
		Simulation::CellParticle banked_particle = local_fission_bank.back();
		local_fission_bank.pop_back();
		if(banked_particle.second.sta() != Particle::BANK) continue;

		/* Split particle */
		double amp = banked_particle.second.wgt() / keff;
		int split = std::max(1,(int)(amp));
		/* New weight of the particle */
		banked_particle.second.wgt() = amp/(double)split;
		/* Put the split particle into the "simulation" list */
		banked_particle.second.sta() = Particle::ALIVE;
		for(int i = 0 ; i < split ; i++)
			fission_bank.push_back(banked_particle);
	}

}

}
}



