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

namespace Helios {

	template<int axis>
	class CylinderOnAxis: public Helios::Surface {

		/* Static constructor functions */
		static Surface* xAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxis<xaxis>(surid,coeffs,flags);
		}
		static Surface* yAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxis<yaxis>(surid,coeffs,flags);
		}
		static Surface* zAxisConstructor(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) {
			return new CylinderOnAxis<zaxis>(surid,coeffs,flags);
		}

		/* Name of the surface */
		std::string name() const;
		/* Print surface internal data */
		void print(std::ostream& out) const;
		/* Evaluate function */
		double function(const Coordinate& pos) const;
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

	    CylinderOnAxis(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags);

		void normal(const Coordinate& point, Direction& vnormal) const;
		bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const;
		Surface* transformate(const Direction& trans) const;

		virtual ~CylinderOnAxis() {/* */};

	};

	/* Name of the surface */
	template<int axis>
	std::string CylinderOnAxis<axis>::name() const {
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
	CylinderOnAxis<axis>::CylinderOnAxis(const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags)
		: Surface(surid,flags) {
		/* Check number of parameters */
		if(coeffs.size() == 1) {
			/* Get the radius */
			radius = coeffs[0];
			point = Coordinate(0,0,0);
		} else if(coeffs.size() == 3) {
			/* Get the radius */
			radius = coeffs[0];
			/* Get point */
			point[axis] = 0.0;
			size_t k = 0;
			for(size_t i = 0 ; i < 3 ; i++) {
				if(i != axis) {
					point[i] = coeffs[k + 1];
					k++;
				}
			}
		} else {
			throw Surface::BadSurfaceCreation(surid,"Bad number of coefficients");
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
	void CylinderOnAxis<axis>::normal(const Coordinate& point, Direction& vnormal) const {
	    /* We now this cylinder is centered on the origin (by definition) */
		vnormal = point;
	    /* Zero component that's not on the axis */
	    vnormal[axis] = 0.0;
	    /* Normalize */
	    vnormal /= radius;
	}

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

	/* Evaluate function */
	template<int axis>
	double CylinderOnAxis<axis>::function(const Coordinate& pos) const {
		Coordinate trpos(pos - point);
		return dotProduct<axis>(trpos, trpos) - radius * radius;
	}

	template<int axis>
	bool CylinderOnAxis<axis>::intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const {
		/* Calculate "quadratic" coefficients */
		double a = 1 - dir[axis] * dir[axis];
	    double k = dotProduct<axis>(dir, pos);
	    double c = dotProduct<axis>(pos, pos) - radius*radius;
	    return quadraticIntersect(a,k,c,sense,distance);
	};

	template<int axis>
	Surface* CylinderOnAxis<axis>::transformate(const Direction& trans) const {
		Coordinate new_point = point + trans;
		return new CylinderOnAxis<axis>(this->getUserId(),this->getFlags(),this->radius,new_point);
	}

} /* namespace Helios */
#endif /* CYLINDERONAXIS_HPP_ */
