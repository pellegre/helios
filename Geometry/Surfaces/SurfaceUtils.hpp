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

#ifndef SURFACEUTILS_HPP_
#define SURFACEUTILS_HPP_

#include <cmath>

#include "../Surface.hpp"

namespace Helios {

	/* Calculate the intersection of a surface with calculated quadratic values. */
	static inline bool quadraticIntersect(const double& a, const double& k, const double& c, const bool& sense, double& distance) {
		/* Discriminant */
		double disc = k*k - a*c;

		if(disc >= 0.0) {
			/* Particle is inside the surface (negative orientation) */
			if (not sense) {
				/* Headed away from the surface */
				if (k <= 0) {
					/* Surface is curving upward */
					if (a > 0) {
						distance = (std::sqrt(disc) - k)/a;
						return true;
					}
					/* Surface curving away and headed in, never hits it */
					else {
						distance = 0.0;
						return false;
					}
				}
				/* Particle is heading toward the surface */
				else {
					distance = std::max(0.0, -c/(std::sqrt(disc) + k));
					return true;
				}
			}
			/* Particle is outside the surface */
			else {
				/* Headed away from the surface */
				if (k >= 0) {
					if (a >= 0) {
						distance = 0.0;
						return false;
					}
					else {
						distance = -(std::sqrt(disc) + k)/a;
						return true;
					}
				}
				else {
					distance = std::max(0.0, c/(std::sqrt(disc) - k));
					return true;
				}
			}
		}

		/* No intercept */
		distance = 0.0;
		return false;
	}

	/* Dot product, ignoring the cylinder axis component */
	template<int cylaxis>
	static inline double dotProduct(const Coordinate& x,const Coordinate& y) {
		/* Axis */
		switch(cylaxis) {
		case xaxis :
			return (x[yaxis] * y[yaxis] + x[zaxis] * y[zaxis]);
			break;
		case yaxis :
			return (x[xaxis] * y[xaxis] + x[zaxis] * y[zaxis]);
			break;
		case zaxis :
			return (x[xaxis] * y[xaxis] + x[yaxis] * y[yaxis]);
			break;
		}
		return 0;
	}

}


#endif /* SURFACEUTILS_HPP_ */
