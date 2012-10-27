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
using namespace Helios;
using namespace OpenMp;

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
		random.jump(i * max_rng_per_source);
		/* Sample particle */
		Particle particle = source->sample(random);
		const Cell* cell(geometry->findCell(particle.pos()));
		fission_bank[i] = CellParticle(cell->getInternalId(),particle);
	}

	/* Jump on base stream of RNGs */
	base.jump(particles_number * max_rng_per_source);
}

void KeffSimulation::launch() {

	/* --- Population */
	double total_population = 0.0;

	/* --- Local particle bank for this simulation */
	vector<CellParticle> local_fission_bank(fission_bank);

	#pragma omp parallel
	{
		/* Initialize some auxiliary variables */
		Surface* surface(0);  /* Surface pointer */
		bool sense(true);     /* Sense of the surface we are crossing */
		double distance(0.0); /* Distance to closest surface */

		/* Local counter */
		double population = 0.0;

		#pragma omp for
		for(size_t i = 0 ; i < fission_bank.size() ; ++i) {

			/* Random number stream for this particle */
			Random r(base);
			r.jump(i * max_rng_per_history);

			/* Flag if particle is out of the system */
			bool outside = false;

			/* 1. ---- Initialize particle from source (get particle from the bank) */
			CellParticle pc = fission_bank[i];
			const Cell* cell = geometry->getCells()[pc.first];
			Particle particle = pc.second;

			while(true) {

				/* 2. ---- Get material and mean free path */
				const Material* material = cell->getMaterial();
				double mfp = material->getMeanFreePath(particle.erg());

				/* 3. ---- Get next surface's distance */
				cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

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
					cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

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
					/* Absorption reaction , we should check if this is a fission reaction */
					if(isotope->isFissile()) {
						double fission = isotope->getFissionProb(particle.erg());
						if(prob > (absorption - fission)) {
							/* We should bank the particle state after simulating the fission reaction */
							isotope->fission(particle,r);
							particle.sta() = Particle::BANK;
							population += particle.wgt();
							local_fission_bank[i] = CellParticle(cell->getInternalId(),particle);
						}
					}
					/* Kill the particle, this is an analog simulation */
					break;
				} else
					/* Scatter with isotope */
					isotope->scatter(particle,r);
			}
		}

		/* Update global population counter */
		#pragma omp critical
		{
			total_population += population;
		}

	}

	/* Jump on random number generation */
	base.jump(fission_bank.size() * max_rng_per_history);

	/* --- Calculate multiplication factor for this cycle */
	keff = total_population / (double) particles_number;

	/* --- Clear particle bank*/
	fission_bank.clear();

	/* --- Re-populate the particle bank with the new source */
	for(size_t i = 0 ; i < local_fission_bank.size() ; ++i) {
		/* Get banked particle */
		Simulation::CellParticle banked_particle = local_fission_bank[i];
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




