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

#ifndef SPHEREONORIGIN_HPP_
#define SPHEREONORIGIN_HPP_

#include "../Surface.hpp"

namespace Helios {

class SphereOnOrigin: public Helios::Surface {
	/* Static constructor functions */
	static Surface* Constructor(const SurfaceObject* definition) {
		return new SphereOnOrigin(definition);
	}
	/* Print surface internal data */
	void print(std::ostream& out) const {
		out << "radius = " << radius;
	}
	/* Return constructor function */
	Surface::Constructor constructor() const {
		return SphereOnOrigin::Constructor;
	}

    /* Sphere radius */
    double radius;
public:
	/* Default, used only on factory */
	SphereOnOrigin() : radius(0) {/* */};
	SphereOnOrigin(const SurfaceId& surid, const SurfaceInfo& flags, const double& radius)
                   : Surface(surid,flags), radius(radius) {/* */};
	SphereOnOrigin(const SurfaceObject* definition);

	void normal(const Coordinate& point, Direction& vnormal) const;
	bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const;
	Surface* transformate(const Direction& trans) const;

	/* Evaluate function */
	double function(const Coordinate& pos) const;

	/* Name of the surface */
	std::string getName() const {
		return "so";
	}

	/* Comparison */
	bool compare(const Surface& sur) const {
        /* safe to static cast because Surface::== already confirmed the type */
        const SphereOnOrigin& sph = static_cast<const SphereOnOrigin&>(sur);
        return (compareFloating(radius,sph.radius));
	}

	virtual ~SphereOnOrigin() {/* */};
};

} /* namespace Helios */
#endif /* SPHEREONORIGIN_HPP_ */
