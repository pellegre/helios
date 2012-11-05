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

#include "Particle.hpp"

namespace Helios {

/* Set an isotropic angle to the particle */
void isotropicDirection(Direction& dir, Random& random) {
	double rand1, rand2, rand3, c1, c2;

	/* Use the rejection method described in Lux & Koblinger, pp. 21-22. */
	do {
		rand1 = 2.0*random.uniform() - 1.0;
		rand2 = 2.0*random.uniform() - 1.0;
		c1 = rand1*rand1 + rand2*rand2;
    }
	while (c1 > 1.0);

	rand3 = 2.0*random.uniform() - 1.0;

	c2 = sqrt(1 - rand3*rand3);

	dir[0] = c2*(rand1*rand1 - rand2*rand2)/c1;
	dir[1] = c2*2.0*rand1*rand2/c1;
	dir[2] = rand3;
}

void azimutalRotation(double mu, Direction& dir, Random& random) {
	/* Remember incident direction cosines */
	Direction diro(dir);
	/* Auxiliary variables */
	double c1, c2, rnd1, rnd2;

	/* Sample like in MCNP (MCNP4C manual p. 2-38). */
	if ((c1 = 1.0 - diro[zaxis]*diro[zaxis]) > 1.0e-09) {

		/* Select two random numbers using the rejection criterion. */
		do {
			rnd1 = 1.0 - 2.0*random.uniform();
			rnd2 = 1.0 - 2.0*random.uniform();
		} while ((c2 = rnd1*rnd1 + rnd2*rnd2) > 1.0);

		double c3 = sqrt((1.0 - mu*mu)/(c1*c2));

		dir[xaxis] = diro[xaxis]*mu + c3*(rnd1*diro[xaxis]*diro[zaxis] - rnd2*diro[yaxis]);
		dir[yaxis] = diro[yaxis]*mu + c3*(rnd1*diro[yaxis]*diro[zaxis] + rnd2*diro[xaxis]);
		dir[zaxis] = diro[zaxis]*mu - rnd1*c1*c3;

	} else {
		/* Same as previous but swap v and w */
		c1 = 1.0 - diro[yaxis]*diro[yaxis];

		/* Select two random numbers using the rejection criterion. */
		do {
			rnd1 = 1.0 - 2.0*random.uniform();
			rnd2 = 1.0 - 2.0*random.uniform();
		} while ((c2 = rnd1*rnd1 + rnd2*rnd2) > 1.0);

		double c3 = sqrt((1.0 - mu*mu)/(c1*c2));

		dir[xaxis] = diro[xaxis]*mu + c3*(rnd1*diro[xaxis]*diro[yaxis] - rnd2*diro[zaxis]);
		dir[yaxis] = diro[zaxis]*mu + c3*(rnd1*diro[zaxis]*diro[yaxis] + rnd2*diro[xaxis]);
		dir[zaxis] = diro[yaxis]*mu - rnd1*c1*c3;
	}
}

std::ostream& operator<<(std::ostream& out, const Particle& q) {
	out << "pos = " << q.position << " ; ";
	out << "dir = " << q.direction << " ; ";
	out << "energy = " << q.energy.second << " (index = " << q.energy.first << ") ; ";
	out << "weight = " << q.weight << " ; ";
	out << "state = " << q.state;
	return out;
}
std::ostream& operator<<(std::ostream& out, const Energy& q) {
	out << "(" << q.first << " , " << q.first << ")";
	return out;
}

} /* namespace Helios */
