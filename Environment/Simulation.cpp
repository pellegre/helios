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

#include "Simulation.hpp"

using namespace std;

namespace Helios {

Simulation::Simulation(const Random& base,McEnvironment* environment) : base(base), environment(environment) {
	/* Parameters for random number on simulations */
	max_rng_per_source = 100;
	max_rng_per_history = 100000;
};

KeffSimulation::KeffSimulation(const Random& _random, McEnvironment* _environment, double keff, size_t _particles_number) :
		Simulation(_random,_environment), keff(keff), particles_number(_particles_number) {

	/* Get the source from the environment */
	const Source* source = environment->getModule<Source>();

	/* Get geometry from the environment */
	geometry = environment->getModule<Geometry>();

	/* Reserve space for the particle bank */
	fission_bank.reserve(2 * particles_number);
	fission_bank.resize(particles_number);

	/* Populate the particle bank with the initial source */
	#pragma omp parallel for
	for(size_t i = 0 ; i < particles_number ; ++i) {
		/* Jump random number generator */
		Random random(base);
		random.getEngine().jump(i * max_rng_per_source);
		/* Sample particle */
		Particle particle = source->sample(random);
		const Cell* cell(geometry->findCell(particle.pos()));
		fission_bank[i] = CellParticle(cell->getInternalId(),particle);
	}

	/* Jump on base stream of RNGs */
	base.getEngine().jump(particles_number * max_rng_per_source);
};

void KeffSimulation::launch() {

	/* --- Population */
	vector<double> population;
	double pop = 0;

	/* --- Local particle bank for for this simulation */
	vector<CellParticle> local_fission_bank(fission_bank);

	#pragma omp parallel
	{
		/* --- Initialize geometry stuff */
		Surface* surface(0);  /* Surface pointer */
		bool sense(true);     /* Sense of the surface we are crossing */
		double distance(0.0); /* Distance to closest surface */

		/* Thread id */
		int bank_id = 0;

		/* Update local containers */
		#pragma omp critical
		{
			/* Initialize local thread data */
			population.push_back(0.0);
			/* Get id of this thread */
			bank_id = population.size() - 1;
		}

		#pragma omp for
		for(size_t i = 0 ; i < fission_bank.size() ; ++i) {

			/* --- Random number */
			Random r(base);
			r.getEngine().jump(i * max_rng_per_history);

			/* Get particle from the user supplied bank */
			CellParticle pc = fission_bank[i];
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
				double collision_distance = -log(r.uniform())*mfp;

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
					collision_distance = -log(r.uniform())*mfp;
				}

				if(out) break;

				/* If we are out, we reach some point inside a cell were a collision should occur */
				particle.pos() = particle.pos() + collision_distance * particle.dir();

				/* get and apply reaction to the particle */
				Reaction* reaction = material->getReaction(particle.eix(),r);
				(*reaction)(particle,r);

				if(particle.sta() == Particle::DEAD) break;
				if(particle.sta() == Particle::BANK) {
					/* Update local particle bank */
					population[bank_id] += particle.wgt();
					local_fission_bank[i] = CellParticle(cell->getInternalId(),particle);
					break;
				}
			}
		}
	}

	/* Jump on random number generation */
	base.getEngine().jump(fission_bank.size() * max_rng_per_history);

	/* --- Calculate multiplication factor for this cycle */

	for(size_t i = 0 ; i < population.size() ; ++i)
		pop += population[i];
	keff = pop / (double) particles_number;

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

} /* namespace Helios */
