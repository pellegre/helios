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

#ifndef SPATIAL_HPP_
#define SPATIAL_HPP_

#include "Distribution.hpp"

namespace Helios {

	class Uniform {
	protected:
		/* Points defining the interval */
		double umin,umax;
	public:
		Uniform() : umin(0), umax(0) {/* */}
		Uniform(const double& umin, const double& umax) : umin(umin), umax(umax) {/* */}
		double getMin() const {return umin;}
		double getMax() const {return umax;}
		void operator() (double& value, Random& r) const {
			value += (umax - umin)*r.uniform() + umin;
		};
		~Uniform() {/* */}
	};

	template<int axis>
	class Box1D : public Distribution {
		Uniform uniform;
		/* Static constructor functions */
		static DistributionBase* xAxisConstructor(const DistributionBaseObject* definition) {
			return new Box1D<xaxis>(definition);
		}
		static DistributionBase* yAxisConstructor(const DistributionBaseObject* definition) {
			return new Box1D<yaxis>(definition);
		}
		static DistributionBase* zAxisConstructor(const DistributionBaseObject* definition) {
			return new Box1D<zaxis>(definition);
		}
		Constructor constructor() const {
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
		/* Name of the distribution */
		std::string getName() const {
			return "box-" + getAxisName<axis>();
		}
		void print(std::ostream& out) const {
			out << " (umin = " << uniform.getMin() << " , umax = " << uniform.getMax() << ") ";
		};
	public:
		Box1D() {/* */}
		Box1D(const DistributionBaseObject* definition) : Distribution(definition) {
			const DistributionObject* distObject = static_cast<const DistributionObject*>(definition);
			std::vector<double> coeffs = distObject->getCoeffs();
			if(coeffs.size() != 2)
				throw BadDistributionCreation(definition->getUserId(),
						"Bad number of coefficients. Expected 2 : umin umax");
			uniform = Uniform(coeffs[0],coeffs[1]);
		}
		virtual void operator() (Particle& particle, Random& r) const {
			uniform(particle.pos()[axis],r);
		};
		~Box1D() {/* */}
	};

	template<int axis>
	class Box2D : public Distribution {
		/* Uniform distributions */
		Uniform uniform1,uniform2;
		/* Static constructor functions */
		static DistributionBase* xAxisConstructor(const DistributionBaseObject* definition) {
			return new Box2D<xaxis>(definition);
		}
		static DistributionBase* yAxisConstructor(const DistributionBaseObject* definition) {
			return new Box2D<yaxis>(definition);
		}
		static DistributionBase* zAxisConstructor(const DistributionBaseObject* definition) {
			return new Box2D<zaxis>(definition);
		}
		Constructor constructor() const {
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
		/* Name of the distribution */
		std::string getName() const {
			return "box-" + getPlaneName<axis>();
		}
		void print(std::ostream& out) const {
			out << " (umin = " << uniform1.getMin() << " , umax = " << uniform1.getMax() << " ) ; ";
			out << "(vmin = " << uniform2.getMin() << " , vmax = " << uniform2.getMax() << " ) ";
		};
	public:
		Box2D() {/* */}
		Box2D(const DistributionBaseObject* definition) : Distribution(definition) {
			const DistributionObject* distObject = static_cast<const DistributionObject*>(definition);
			std::vector<double> coeffs = distObject->getCoeffs();
			if(coeffs.size() != 4)
				throw BadDistributionCreation(definition->getUserId(),
						"Bad number of coefficients. Expected 4 : umin umax vmin vmax");
			uniform1 = Uniform(coeffs[0],coeffs[1]);
			uniform2 = Uniform(coeffs[2],coeffs[3]);
		}
		virtual void operator() (Particle& particle, Random& r) const {
			Coordinate& position = particle.pos();
			switch(axis) {
			case xaxis :
				uniform1(position[yaxis],r);
				uniform2(position[zaxis],r);
				break;
			case yaxis :
				uniform1(position[xaxis],r);
				uniform2(position[zaxis],r);
				break;
			case zaxis :
				uniform1(position[xaxis],r);
				uniform2(position[yaxis],r);
				break;
			}
		};
		~Box2D() {/* */}
	};

	class Box3D : public Distribution {
		Uniform uniformx;
		Uniform uniformy;
		Uniform uniformz;
		static DistributionBase* Box3DConstructor(const DistributionBaseObject* definition) {
			return new Box3D(definition);
		}
		Constructor constructor() const {
			return Box3DConstructor;
		}
		/* Name of the distribution */
		std::string getName() const {
			return "box-xyz";
		}
		void print(std::ostream& out) const {
			out << " (xmin = " << uniformx.getMin() << " , xmax = " << uniformx.getMax() << " ) ; ";
			out << "(ymin = " << uniformy.getMin() << " , ymax = " << uniformy.getMax() << " ) ; ";
			out << "(zmin = " << uniformz.getMin() << " , zmax = " << uniformz.getMax() << " ) ";
		};
	public:
		Box3D() {/* */}
		Box3D(const DistributionBaseObject* definition) : Distribution(definition) {
			const DistributionObject* distObject = static_cast<const DistributionObject*>(definition);
			std::vector<double> coeffs = distObject->getCoeffs();
			if(coeffs.size() != 6)
				throw BadDistributionCreation(definition->getUserId(),
						"Bad number of coefficients. Expected 6 : xmin xmax ymin ymax zmin zmax");
			uniformx = Uniform(coeffs[0],coeffs[1]);
			uniformy = Uniform(coeffs[2],coeffs[3]);
			uniformz = Uniform(coeffs[4],coeffs[5]);
		}
		virtual void operator() (Particle& particle, Random& r) const {
			uniformx(particle.pos()[xaxis],r);
			uniformy(particle.pos()[yaxis],r);
			uniformz(particle.pos()[zaxis],r);
		};
		~Box3D() {/* */}
	};

	template<int axis>
	class Cyl2D : public Distribution {
		double rmin,rmax;
		/* Static constructor functions */
		static DistributionBase* xAxisConstructor(const DistributionBaseObject* definition) {
			return new Cyl2D<xaxis>(definition);
		}
		static DistributionBase* yAxisConstructor(const DistributionBaseObject* definition) {
			return new Cyl2D<yaxis>(definition);
		}
		static DistributionBase* zAxisConstructor(const DistributionBaseObject* definition) {
			return new Cyl2D<zaxis>(definition);
		}
		Constructor constructor() const {
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
		/* Name of the distribution */
		std::string getName() const {
			return "cyl-" + getAxisName<axis>();
		}
		void print(std::ostream& out) const {
			out << " (rmin = " << rmin << " , rmax = " << rmax << ") ";
		};
	public:
		Cyl2D() {/* */}
		Cyl2D(const DistributionBaseObject* definition) : Distribution(definition) {
			const DistributionObject* distObject = static_cast<const DistributionObject*>(definition);
			std::vector<double> coeffs = distObject->getCoeffs();
			if(coeffs.size() != 2)
				throw BadDistributionCreation(definition->getUserId(),
						"Bad number of coefficients. Expected 2 : rmin rmax");
			rmin = coeffs[0];
			rmax = coeffs[1];
		}
		virtual void operator() (Particle& particle, Random& r) const {
			Coordinate& position = particle.pos();
			/* Get angle */
			double value = r.uniform();
			/* Get radius */
			double radius =  sqrt( rmin * rmin + (rmax * rmax - rmin * rmin) * r.uniform());
			switch(axis) {
			case xaxis :
				position[yaxis] += radius * cos(2 * M_PI * value);
				position[zaxis] += radius * sin(2 * M_PI * value);
				break;
			case yaxis :
				position[xaxis] += radius * cos(2 * M_PI * value);
				position[zaxis] += radius * sin(2 * M_PI * value);
				break;
			case zaxis :
				position[xaxis] += radius * cos(2 * M_PI * value);
				position[yaxis] += radius * sin(2 * M_PI * value);
				break;
			}
		};
		~Cyl2D() {/* */}
	};
}


#endif /* SPATIAL_HPP_ */
