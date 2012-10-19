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

#ifndef CYLINDERONAXIS_HPP_
#define CYLINDERONAXIS_HPP_

#include <cmath>

#include "../Surface.hpp"
#include "SurfaceUtils.hpp"

namespace Helios {

	template<int axis>
	class CylinderOnAxis: public Helios::Surface {

		/* Static constructor functions */
		static Surface* xAxisConstructor(const Definition* definition) {
			return new CylinderOnAxis<xaxis>(definition);
		}
		static Surface* yAxisConstructor(const Definition* definition) {
			return new CylinderOnAxis<yaxis>(definition);
		}
		static Surface* zAxisConstructor(const Definition* definition) {
			return new CylinderOnAxis<zaxis>(definition);
		}

		/* Print surface internal data */
		void print(std::ostream& out) const;
		/* Return constructor function */
		Constructor constructor() const;

	    /* Cylinder radius */
	    double radius;
	    /* Some point through which the cylinder's axis passes (defined only with two values) */
	    Coordinate point;

	public:
		/* Default, used only on factory */
	    CylinderOnAxis() : radius(0) , point(0,0,0) {/* */};
	    CylinderOnAxis(const SurfaceId& surid, const SurfaceInfo& flags, const double& radius, const Coordinate& point)
	                   : Surface(surid,flags), radius(radius) , point(point) {/* */};

	    CylinderOnAxis(const Definition* definition);

		void normal(const Coordinate& point, Direction& vnormal) const;
		bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const;
		Surface* transformate(const Direction& trans) const;
		/* Name of the surface */
		std::string getName() const;

		/* Evaluate function */
		double function(const Coordinate& pos) const;

		/* Comparison */
		bool compare(const Surface& sur) const {
	        /* safe to static cast because Surface::== already confirmed the type */
	        const CylinderOnAxis<axis>& cyl = static_cast<const CylinderOnAxis<axis>&>(sur);
	        return (compareFloating(radius,cyl.radius) && (compareTinyVector(point,cyl.point)));
		}

		virtual ~CylinderOnAxis() {/* */};

	};

	/* Name of the surface */
	template<int axis>
	std::string CylinderOnAxis<axis>::getName() const {
		return "c/" + getAxisName<axis>();
	}

	/* Constructor */
	template<int axis>
	CylinderOnAxis<axis>::CylinderOnAxis(const Definition* definition)
		: Surface(definition) {
		/* Check number of parameters */
		if(definition->getCoeffs().size() == 3) {
			/* Get the radius */
			radius = definition->getCoeffs()[0];
			/* Get point */
			point[axis] = 0.0;
			size_t k = 0;
			for(size_t i = 0 ; i < 3 ; i++) {
				if(i != axis) {
					point[i] = definition->getCoeffs()[k + 1];
					k++;
				}
			}
		} else {
			throw Surface::BadSurfaceCreation(definition->getUserSurfaceId(),
				  "Bad number of coefficients. Expected 3 values : radius pos1 pos2");
		}
	}

	/* Print surface internal data */
	template<int axis>
	void CylinderOnAxis<axis>::print(std::ostream& out) const {
		out << "radius = " << radius << " ; ";
		out << "point = " << point;
	}

	template<int axis>
	Surface::Constructor CylinderOnAxis<axis>::constructor() const {
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
	void CylinderOnAxis<axis>::normal(const Coordinate& position, Direction& vnormal) const {
	    /* We now this cylinder is centered on the origin (by definition) */
		vnormal = position - point;
	    /* Zero component that's not on the axis */
	    vnormal[axis] = 0.0;
	    /* Normalize */
	    vnormal /= radius;
	}

	/* Evaluate function */
	template<int axis>
	double CylinderOnAxis<axis>::function(const Coordinate& position) const {
		Coordinate trpos(position - point);
		return dotProduct<axis>(trpos, trpos) - radius * radius;
	}

	template<int axis>
	bool CylinderOnAxis<axis>::intersect(const Coordinate& position, const Direction& dir, const bool& sense, double& distance) const {
		/* Calculate "quadratic" coefficients */
		double a = 1 - dir[axis] * dir[axis];
	    Coordinate trpos(position - point);
	    double k = dotProduct<axis>(dir, trpos);
	    double c = dotProduct<axis>(trpos, trpos) - radius*radius;
	    return quadraticIntersect(a,k,c,sense,distance);
	};

	template<int axis>
	Surface* CylinderOnAxis<axis>::transformate(const Direction& trans) const {
		Coordinate new_point = point + trans;
		return new CylinderOnAxis<axis>(this->getUserId(),this->getFlags(),this->radius,new_point);
	}

} /* namespace Helios */
#endif /* CYLINDERONAXIS_HPP_ */
