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
	fission_bank.resize(particles_number);

	/* Populate the particle bank with the initial source */
	#pragma omp parallel for
	for(size_t i = 0 ; i < particles_number ; ++i) {
		/* Jump random number generator */
		Random random(base);
		random.jump(i * Source::max_samples);
		fission_bank[i].push_back(source->sample(random));
	}

	/* Jump on base stream of RNGs */
	base.jump(particles_number * Source::max_samples);
}

void KeffSimulation::launch() {

	/* --- Population */
	double total_population = 0.0;

	#pragma omp parallel
	{
		#pragma omp for
		for(size_t i = 0 ; i < fission_bank.size() ; ++i) {
			/* Simulate cycle */
			double population = cycle(i);
			/* Update global population counter */
			#pragma omp critical
			{
				total_population += population;
			}
		}

	}

	/* --- Calculate multiplication factor for this cycle */
	keff = total_population / (double) particles_number;

}




