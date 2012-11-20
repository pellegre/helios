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

KeffSimulation::KeffSimulation(const Random& _random, McEnvironment* _environment, double _keff, size_t _particles_number) :
		CriticalitySimulation(_random,_environment,_keff,_particles_number) {

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
		random.jump(i * Source::max_samples);
		fission_bank[i] = source->sample(random);
	}

	/* Jump on base stream of RNGs */
	base.jump(particles_number * Source::max_samples);
}

void KeffSimulation::launch() {

	/* --- Population */
	double total_population = 0.0;

	/* --- Local particle bank for this simulation */
	vector<vector<CellParticle> > local_bank(fission_bank.size());

	#pragma omp parallel
	{
		/* Local counter */
		double population = 0.0;

		#pragma omp for
		for(size_t i = 0 ; i < fission_bank.size() ; ++i)
			population += cycle(i,local_bank[i]);

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
	for(vector<vector<CellParticle> >::const_iterator it = local_bank.begin() ; it != local_bank.end() ; ++it) {
		/* Loop over local bank */
		for(vector<CellParticle>::const_iterator it_particle = (*it).begin() ; it_particle != (*it).end() ; ++it_particle) {
			/* Get banked particle */
			CellParticle& banked_particle = (*it_particle);
			/* Split particle */
			double amp = banked_particle.second.wgt() / keff;
			int split = std::max(1,(int)(amp));
			/* New weight of the particle */
			banked_particle.second.wgt() = amp/(double)split;
			/* Put the split particle into the "simulation" list */
			for(int i = 0 ; i < split ; i++)
				fission_bank.push_back(banked_particle);
		}
	}

}




