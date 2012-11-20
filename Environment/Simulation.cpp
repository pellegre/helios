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

CriticalitySimulation::CriticalitySimulation(const Random& _random, McEnvironment* _environment, double keff, size_t _particles_number) :
		Simulation(_random,_environment), keff(keff), particles_number(_particles_number), geometry(environment->getModule<Geometry>()) {
}

double CriticalitySimulation::cycle(size_t nbank) {
	/* For each bank, we should simulate the banked particles */
	vector<CellParticle> local_bank = fission_bank[nbank];

	/* Check size of the local bank */
	if(local_bank.size() == 0) return 0.0;

	/* Clear global bank */
	fission_bank[nbank].clear();

	/* Local counter */
	double population = 0.0;

	/* Random number stream for this particle */
	Random r(base);
	r.jump(nbank * max_rng_per_history);

	/* Loop over each particle */
	for(vector<CellParticle>::const_iterator it = local_bank.begin() ; it != local_bank.end() ; ++it) {

		/* Flag if particle is out of the system */
		bool outside = false;

		/* 1. ---- Initialize particle from source (get particle from the bank) */
		CellParticle pc = (*it);
		/* Split particle */
		double amp = (*it).second.getWeight() / keff;
		int split = std::max(1,(int)(amp));

		/* Simulate each particle */
		for(int i = 0 ; i < split ; ++i) {

			/* Initialize some auxiliary variables */
			Surface* surface(0);  /* Surface pointer */
			bool sense(true);     /* Sense of the surface we are crossing */
			double distance(0.0); /* Distance to closest surface */

			/* Set initial conditions */
			const Cell* cell = pc.first;
			Particle particle = pc.second;
			/* New weight of the particle */
			particle.wgt() = amp/(double)split;

			while(true) {

				/* 2. ---- Get material and mean free path */
				const Material* material = cell->getMaterial();
				double mfp = material->getMeanFreePath(particle.erg());

				/* 3. ---- Get next surface's distance */
				cell->intersect(particle.pos(), particle.dir(), surface, sense, distance);

				/* 4. ---- Get collision distance */
				double collision_distance = -log(r.uniform())*mfp;

				/* 5. ---- Check sampled distance against closest surface distance */
				while(collision_distance > distance) {

					/* 5.1 ---- Transport the particle to the surface */
					particle.pos() = particle.pos() + distance * particle.dir();

					/* 5.2 ---- Cross the surface (checking boundary conditions) */
					outside = not surface->cross(particle,sense,cell);
					assert(cell != 0);
					if(outside) break;

					/* 5.3 ---- Get material of the current cell (after crossing the surface) */
					material = cell->getMaterial();
					/* Mean free path (the particle didn't change the energy) */
					mfp = material->getMeanFreePath(particle.erg());

					/* 5.4 ---- Get next surface's distance */
					cell->intersect(particle.pos(), particle.dir(), surface, sense, distance);

					/* 5.5 ---- Get collision distance */
					collision_distance = -log(r.uniform())*mfp;
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
							/* We should bank the particle state after simulating the fission reaction */
							Reaction* fission_reaction = isotope->fission();
							(*fission_reaction)(particle, r);
							population += particle.wgt();
							fission_bank[nbank].push_back(CellParticle(cell,particle));
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
		}

	}

	/* Return population */
	return population;
}

Simulation::Simulation(const Random& base,McEnvironment* environment) : base(base), environment(environment) {
	/* Parameters for random number on simulations */
	max_rng_per_history = 1000000;
}

} /* namespace Helios */
