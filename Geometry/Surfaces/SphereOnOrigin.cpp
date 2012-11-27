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

#include "SurfaceUtils.hpp"
#include "SphereOnOrigin.hpp"

namespace Helios {

SphereOnOrigin::SphereOnOrigin(const SurfaceObject* definition) : Surface(definition) {
	/* Check number of parameters */
	if(definition->getCoeffs().size() == 1) {
		/* Get the radius */
		radius = definition->getCoeffs()[0];
	} else {
		throw Surface::BadSurfaceCreation(definition->getUserSurfaceId(),
			  "Bad number of coefficients. Expected 1 value : radius ");
	}
}

void SphereOnOrigin::normal(const Coordinate& point, Direction& vnormal) const {
	vnormal = point / dot(point, point);
}

bool SphereOnOrigin::intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const {
	/* Calculate "quadratic" coefficients */
	double a = 1.0;
    double k = dot(pos, dir);
    double c = dot(pos, pos) - radius*radius;
	return quadraticIntersect(a,k,c,sense,distance);
}

Surface* SphereOnOrigin::transformate(const Direction& trans) const {
	return new SphereOnOrigin(this->getUserId(),this->getFlags(),this->radius);
}

/* Evaluate function */
double SphereOnOrigin::function(const Coordinate& pos) const {
	return dot(pos, pos) - radius*radius;
}

} /* namespace Helios */
