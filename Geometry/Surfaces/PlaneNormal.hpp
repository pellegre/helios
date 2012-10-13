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

#ifndef PLANENORMAL_HPP_
#define PLANENORMAL_HPP_

#include "../Surface.hpp"

namespace Helios {

	template<int axis>
	class PlaneNormal: public Helios::Surface {

		/* Static constructor functions */
		static Surface* xAxisConstructor(const Definition* definition) {
			return new PlaneNormal<xaxis>(definition);
		}
		static Surface* yAxisConstructor(const Definition* definition) {
			return new PlaneNormal<yaxis>(definition);
		}
		static Surface* zAxisConstructor(const Definition* definition) {
			return new PlaneNormal<zaxis>(definition);
		}

		/* Print surface internal data */
		void print(std::ostream& out) const;
		/* Return constructor function */
		Constructor constructor() const;

		/* Cylinder radius */
		double coordinate;

	public:
		/* Default, used only on factory */
		PlaneNormal() : coordinate(0) {/* */};
		PlaneNormal(const SurfaceId& surid, const SurfaceInfo& flags, const double& coordinate)
					   : Surface(surid,flags), coordinate(coordinate) {/* */};

		PlaneNormal(const Definition* definition);

		void normal(const Coordinate& point, Direction& vnormal) const;
		bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const;
		Surface* transformate(const Direction& trans) const;
		/* Name of the surface */
		std::string name() const;
		/* Evaluate function */
		double function(const Coordinate& pos) const;

		/* Comparison */
		bool compare(const Surface& sur) const {
	        /* safe to static cast because Surface::== already confirmed the type */
	        const PlaneNormal<axis>& plane = static_cast<const PlaneNormal<axis>&>(sur);
	        return compareFloating(coordinate,plane.coordinate);
		}

		virtual ~PlaneNormal() {/* */};
	};

	/* Name of the surface */
	template<int axis>
	std::string PlaneNormal<axis>::name() const {
		return "p" + getAxisName<axis>();

	}

	/* Constructor */
	template<int axis>
	PlaneNormal<axis>::PlaneNormal(const Definition* definition)
		: Surface(definition) {
		/* Check number of parameters */
		if(definition->getCoeffs().size() == 1) {
			/* Get the radius */
			coordinate = definition->getCoeffs()[0];
		} else {
			throw Surface::BadSurfaceCreation(definition->getUserSurfaceId(),
				  "Bad number of coefficients. Expected 1 : coordinate");
		}
	}

	/* Print surface internal data */
	template<int axis>
	void PlaneNormal<axis>::print(std::ostream& out) const {
		out << "coordinate = " << coordinate;
	}

	template<int axis>
	Surface::Constructor PlaneNormal<axis>::constructor() const {
		switch(axis) {
		case xaxis :
			return xAxisConstructor;
			break;
		case yaxis :
			return yAxisConstructor;
			break;
		case zaxis :
			return zAxisConstructor;
			break;
		}
		return 0;
	}

	template<int axis>
	void PlaneNormal<axis>::normal(const Coordinate& point, Direction& vnormal) const {
		vnormal = 0.0;
		vnormal[axis] = 1.0;
	}

	/* Evaluate function */
	template<int axis>
	double PlaneNormal<axis>::function(const Coordinate& position) const {
		return position[axis] - coordinate;
	}

	template<int axis>
	bool PlaneNormal<axis>::intersect(const Coordinate& position, const Direction& dir, const bool& sense, double& distance) const {
	    if (((sense == false) && (dir[axis] > 0)) || ((sense == true)  && (dir[axis] < 0))) {
	        /* Headed towards surface */
	        distance = (coordinate - position[axis]) / dir[axis];
	        distance = std::max(0.0, distance);
	        return true;
	    }
	    distance = 0.0;
	    return false;
	};

	template<int axis>
	Surface* PlaneNormal<axis>::transformate(const Direction& trans) const {
		return new PlaneNormal<axis>(this->getUserId(),this->getFlags(),(this->coordinate + trans[axis]));
	}

} /* namespace Helios */
#endif /* PLANENORMAL_HPP_ */
