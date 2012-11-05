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

#include "ElasticScattering.hpp"

namespace Helios {

using namespace AceReaction;

static double ElasticScattering::energy_freegas_threshold = 400.0; /* By default, 400.0 kT*/
static double ElasticScattering::awr_freegas_threshold = 1.0;      /* By default, only H */

void ElasticScattering::targetVelocity(double energy, Direction direction, Direction& velocity, Random& random) {

	/* Compare to threshold criteria */
	if ((energy > energy_freegas_threshold*temperature) && (awr > awr_freegas_threshold)) {
		/* Target velocity insignificant, set components to zero */
		velocity = Direction(0.0,0.0,0.0);
		return;
	}

	/*
	 * Rejection algorithm extracted from MCNP5 manual. Samples target
	 * energy z2/ar, cosine between target and neutron velocity c.
	 */
	double ar = awr/temperature;
	double ycn = sqrt(energy*ar);

	/* Auxiliary variables */
	double r1, z2, rnd1, rnd2, s, x2, c;

	/* Rejection sampling */
	do {
		if (random.uniform()*(ycn + 1.12837917) > ycn) {
			r1 = random.uniform();
			z2 = -log(r1*random.uniform());
		}
		else {
			do {
				rnd1 = random.uniform();
				rnd2 = random.uniform();

				r1 = rnd1*rnd1;
				s = r1 + rnd2*rnd2;
			}
			while (s > 1.0);

			z2 = -r1*log(s)/s - log(random.uniform());
		}

		double z = sqrt(z2);
		c = 2.0*random.uniform() - 1.0;
		x2 = ycn*ycn + z2 - 2*ycn*z*c;
		rnd1 = random.uniform()*(ycn + z);
	}
	while (rnd1*rnd1 > x2);

	/* Rotate direction cosines */
	azimutalRotation(c, direction, random);

	/* Calculate velocity components */
	double vel = sqrt(z2/ar);
	velocity = vel * direction;
}

void ElasticScattering::operator()(Particle& particle, Random& r) const {

}

} /* namespace Helios */
