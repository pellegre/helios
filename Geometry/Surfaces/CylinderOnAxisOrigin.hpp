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

#ifndef CYLINDERONAXISORIGIN_HPP_
#define CYLINDERONAXISORIGIN_HPP_

#include "../Surface.hpp"
#include "SurfaceUtils.hpp"
#include "CylinderOnAxis.hpp"

namespace Helios {

	template<int axis>
	class CylinderOnAxisOrigin: public Helios::Surface {

		/* Static constructor functions */
		static Surface* xAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxisOrigin<xaxis>(surid,coeffs,flags);
		}
		static Surface* yAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxisOrigin<yaxis>(surid,coeffs,flags);
		}
		static Surface* zAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxisOrigin<zaxis>(surid,coeffs,flags);
		}

		/* Name of the surface */
		std::string name() const;
		/* Print surface internal data */
		void print(std::ostream& out) const;
		/* Return constructor function */
		Constructor constructor() const;

		/* Cylinder radius */
		double radius;

	public:
		/* Default, used only on factory */
		CylinderOnAxisOrigin() : radius(0) {/* */};
		CylinderOnAxisOrigin(const SurfaceId& surid, const SurfaceInfo& flags, const double& radius)
					   : Surface(surid,flags), radius(radius) {/* */};

		CylinderOnAxisOrigin(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags);

		void normal(const Coordinate& point, Direction& vnormal) const;
		bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const;
		Surface* transformate(const Direction& trans) const;
		/* Evaluate function */
		double function(const Coordinate& pos) const;

		/* Comparison */
		bool compare(const Surface& sur) const {
	        /* safe to static cast because Surface::== already confirmed the type */
	        const CylinderOnAxisOrigin<axis>& cyl = static_cast<const CylinderOnAxisOrigin<axis>&>(sur);
	        return compareFloating(radius,cyl.radius);
		}

		virtual ~CylinderOnAxisOrigin() {/* */};
	};

	/* Name of the surface */
	template<int axis>
	std::string CylinderOnAxisOrigin<axis>::name() const {
		switch(axis) {
		case xaxis :
			return "cx";
			break;
		case yaxis :
			return "cy";
			break;
		case zaxis :
			return "cz";
			break;
		}
		return "";
	}

	/* Constructor */
	template<int axis>
	CylinderOnAxisOrigin<axis>::CylinderOnAxisOrigin(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags)
		: Surface(surid,flags) {
		/* Check number of parameters */
		if(coeffs.size() == 1) {
			/* Get the radius */
			radius = coeffs[0];
		} else {
			throw Surface::BadSurfaceCreation(surid,"Bad number of coefficients");
		}
	}

	/* Print surface internal data */
	template<int axis>
	void CylinderOnAxisOrigin<axis>::print(std::ostream& out) const {
		out << "radius = " << radius;
	}

	template<int axis>
	Surface::Constructor CylinderOnAxisOrigin<axis>::constructor() const {
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
	void CylinderOnAxisOrigin<axis>::normal(const Coordinate& point, Direction& vnormal) const {
		/* We now this cylinder is centered on the origin (by definition) */
		vnormal = point;
		/* Zero component that's not on the axis */
		vnormal[axis] = 0.0;
		/* Normalize */
		vnormal /= radius;
	}

	/* Evaluate function */
	template<int axis>
	double CylinderOnAxisOrigin<axis>::function(const Coordinate& position) const {
		return dotProduct<axis>(position, position) - radius * radius;
	}

	template<int axis>
	bool CylinderOnAxisOrigin<axis>::intersect(const Coordinate& position, const Direction& dir, const bool& sense, double& distance) const {
		/* Calculate "quadratic" coefficients */
		double a = 1 - dir[axis] * dir[axis];
		double k = dotProduct<axis>(dir, position);
		double c = dotProduct<axis>(position,position) - radius*radius;
		return quadraticIntersect(a,k,c,sense,distance);
	};

	template<int axis>
	Surface* CylinderOnAxisOrigin<axis>::transformate(const Direction& trans) const {
		if(compareTinyVector(trans,Direction(0,0,0))) {
			return new CylinderOnAxisOrigin<axis>(this->getUserId(),this->getFlags(),this->radius);
		} else {
			return new CylinderOnAxis<axis>(this->getUserId(),this->getFlags(),this->radius,trans);
		}
	}

} /* namespace Helios */
#endif /* CYLINDERONAXISORIGIN_HPP_ */
