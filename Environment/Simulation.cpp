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

KeffSimulation::KeffSimulation(const Random& _random, McEnvironment* _environment, double keff, size_t _particles_number) :
		Simulation(_random,_environment), keff(keff), particles_number(_particles_number),
		initial_source(environment->getModule<Source>()), fission_bank(particles_number) {/* */}

bool nonVoid(const Material*& material, Particle& particle, const Cell*& cell) {
	while(not material) {
		/* Initialize some auxiliary variables */
		Surface* surface(0);  /* Surface pointer */
		bool sense(true);     /* Sense of the surface we are crossing */
		double distance(0.0); /* Distance to closest surface */

		/* Get next surface's distance */
		cell->intersect(particle.pos(), particle.dir(), surface, sense, distance);

		/* Transport the particle to the surface */
		particle.pos() = particle.pos() + distance * particle.dir();

		/*  Cross the surface (checking boundary conditions) */
		bool outside = not surface->cross(particle,sense,cell);
		assert(cell != 0);
		if(outside) return false;

		/* Update material */
		material = cell->getMaterial();
	}
	return true;
}

double KeffSimulation::cycle(size_t nbank) {
	/* Initialize some auxiliary variables */
	Surface* surface(0);  /* Surface pointer */
	bool sense(true);     /* Sense of the surface we are crossing */
	double distance(0.0); /* Distance to closest surface */

	/* Local counter */
	double population = 0.0;

	/* Random number stream for this particle */
	Random r(base);
	r.jump(nbank * max_rng_per_history);

	/* Flag if particle is out of the system */
	bool outside = false;

	/* 1. ---- Initialize particle from source (get particle from the bank) */
	CellParticle& pc = fission_bank[nbank];
	const Cell* cell = pc.first;
	Particle& particle = pc.second;

	while(true) {

		/* 2. ---- Get material and mean free path */
		const Material* material = cell->getMaterial();
		/* Transport the particle until a non-void cell is found (checking boundary conditions) */
		outside = not nonVoid(material, particle, cell);
		if(outside) break;

		/* 3. ---- Get next surface's distance */
		cell->intersect(particle.pos(), particle.dir(), surface, sense, distance);

		/* 4. ---- Get collision distance */
		double mfp = material->getMeanFreePath(particle.erg());
		double collision_distance = -log(r.uniform())*mfp;

		/* 5. ---- Check sampled distance against closest surface distance */
		while(collision_distance >= distance) {
			/* 5.1 ---- Transport the particle to the surface */
			particle.pos() = particle.pos() + distance * particle.dir();

			/* 5.2 ---- Cross the surface (checking boundary conditions) */
			outside = not surface->cross(particle,sense,cell);
			assert(cell != 0);
			if(outside) break;

			/* 5.3 ---- Get material of the current cell (after crossing the surface) */
			const Material* new_material = cell->getMaterial();
			/* Transport the particle until a non-void cell is found (checking boundary conditions) */
			outside = not nonVoid(new_material, particle, cell);
			if(outside) break;

			/* 5.4 ---- Get next surface's distance */
			double new_distance(0.0);
			cell->intersect(particle.pos(), particle.dir(), surface, sense, new_distance);

			/* Check if there is a change on the material */
			if(new_material != material) {
				/* Mean free path (the particle didn't change the energy) */
				mfp = new_material->getMeanFreePath(particle.erg());
				/* 5.5 ---- Get collision distance */
				collision_distance = -log(r.uniform())*mfp;
				/* Update distance */
				distance = new_distance;
				/* Update material */
				material = new_material;
			} else {
				/* 5.5 ---- Get collision distance */
				collision_distance -= distance;
				/* Update distance */
				distance = new_distance;
			}
		}

		if(outside) break;

		/* 6. Move the particle to the collision point */
		particle.pos() = particle.pos() + collision_distance * particle.dir();

		/* 7. ---- Sample isotope */
		const Isotope* isotope = material->getIsotope(particle.erg(),r);

		/* 8. ---- Sample reaction with the isotope */

		/* 8.1 ---- Check the type of reaction reaction */
		double absorption = isotope->getAbsorptionProb(particle.erg());
		double prob = r.uniform();
		if(prob < absorption) {
			/* 8.2 ---- Absorption reaction , we should check if this is a fission reaction */
			if(isotope->isFissile()) {
				double fission = isotope->getFissionProb(particle.erg());
				if(prob > (absorption - fission)) {
					/* Get NU-bar */
					double nubar = isotope->getNuBar(particle.erg()) * particle.wgt() / keff;
					/* Integer part */
					int nu = (int) nubar;
					if (r.uniform() < nubar - (double)nu) nu++;
					/* Get fission reaction */
					Reaction* fission_reaction = isotope->fission();
					/* Accumulate population */
					population += particle.wgt() * nu;
					/* We should bank the particle state after simulating the fission reaction */
					for(int i = 0 ; i < nu ; ++i) {
						Particle new_particle(particle);
						new_particle.wgt() = 1.0;
						/* Apply reaction */
						(*fission_reaction)(new_particle, r);
						local_bank[nbank].push_back(CellParticle(cell,new_particle));
					}
				}
			}
			/* Kill the particle, this is an analog simulation */
			break;
		} else {
			/* Get elastic probability */
			double elastic = isotope->getElasticProb(particle.erg());
			/* 8.2 ---- Sample between inelastic and elastic scattering */
			if((prob - absorption) <= elastic) {
				/* Elastic reaction */
				Reaction* elastic_reaction = isotope->elastic();
				/* Apply the reaction */
				(*elastic_reaction)(particle,r);
			} else {
				/* Scatter with isotope sampling an inelastic reaction*/
				Reaction* inelastic_reaction = isotope->inelastic(particle.erg(),r);
				/* Apply the reaction */
				(*inelastic_reaction)(particle,r);
			}
		}
	}
	/* Return the population */
	return population;
}

void KeffSimulation::source(size_t nbank) {
	/* Jump random number generator */
	Random random(base);
	random.jump(nbank * Source::max_samples);
	CellParticle source_particle = initial_source->sample(random);
	source_particle.second.wgt() = keff;
	fission_bank[nbank] = source_particle;
}

void KeffSimulation::launch() {
	/* Get number of banks */
	size_t nbanks = fission_bank.size();

	/* Resize the local bank before the simulation */
	local_bank.resize(nbanks);

	/* Simulate the current bank */
	double total_population = simulateBank(nbanks);

	/* Jump on random number generation */
	base.jump(fission_bank.size() * max_rng_per_history);

	/* --- Calculate multiplication factor for this cycle */
	keff = total_population / (double) particles_number;

	/* --- Clear particle bank (global) */
	fission_bank.clear();

	/* --- Re-populate the particle bank with the new source */
	for(vector<vector<CellParticle> >::iterator it = local_bank.begin() ; it != local_bank.end() ; ++it)
		/* Loop over local bank */
		for(vector<CellParticle>::iterator it_particle = (*it).begin() ; it_particle != (*it).end() ; ++it_particle)
			/* Get banked particle */
			fission_bank.push_back((*it_particle));

	/* Clear local bank */
	local_bank.clear();
}

/* Random number stride for each history */
size_t Simulation::max_rng_per_history = 10000;

Simulation::Simulation(const Random& base, McEnvironment* environment) : base(base), environment(environment) {/* */}

} /* namespace Helios */
