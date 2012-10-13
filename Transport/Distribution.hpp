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

#ifndef DISTRIBUTION_HPP_
#define DISTRIBUTION_HPP_

#include "../Common/Common.hpp"
#include "Particle.hpp"

namespace Helios {

	/* Base class for distribution */
	class DistributionBase {

	public:
		/* Base class to define a distribution */
		class Definition {
			/* Type of distribution */
			std::string type;
			/* Distribution ID on this problem */
			DistributionId distid;
			/* Coefficients for each child */
			std::vector<double> coeffs;
		public:
			Definition(const std::string& type, const DistributionId& distid, const std::vector<double>& coeffs) :
				type(type), distid(distid), coeffs(coeffs) {/* */}
			DistributionId getUserId() const {
				return distid;
			}
			std::string getType() const {
				return type;
			}
			std::vector<double> getCoeffs() const {
				return coeffs;
			}
			virtual ~Definition() {/* */}
		};

		/* Exception */
		class BadDistributionCreation : public std::exception {
			std::string reason;
		public:
			BadDistributionCreation(const DistributionId& distid, const std::string& msg) {
				reason = "Cannot create distribution " + toString(distid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadDistributionCreation() throw() {/* */};
		};

		/* Constructor */
		typedef DistributionBase(*(*Constructor)(const Definition*));
		/* Friendly factory */
		friend class DistributionFactory;

		/* Constructor from definition */
		DistributionBase(const Definition* definition) : distid(definition->getUserId()) {/* */};
		/* Only used by the factory */
		DistributionBase() : distid() {/* */};

		/* Sample a coordinate */
		virtual void operator() (Particle& particle, Random& r) const = 0;

		virtual ~DistributionBase() {/* */};

		DistributionId getDistid() const {
			return distid;
		}

	protected:
		/* Get name of the distribution */
		virtual std::string getName() const  = 0;
		/* Get constructor */
		virtual Constructor constructor() const = 0;
	private:
		/* Distribution ID defined by the user */
		DistributionId distid;
	};

	class Uniform {
	protected:
		/* Points defining the interval */
		double umin,umax;
	public:
		Uniform() : umin(0), umax(0) {/* */}
		Uniform(const double& umin, const double& umax) : umin(umin), umax(umax) {/* */}
		void operator() (double& value, Random& r) const {
			value += (umax - umin)*r.uniform() + umin;
		};
		~Uniform() {/* */}
	};

	template<int axis>
	class Box1D : public DistributionBase {
		Uniform uniform;
	public:
		Box1D() {/* */}
		Box1D(const Definition* definition) : DistributionBase(definition) {/* */}
		virtual void operator() (Particle& particle, Random& r) const {
			uniform(particle.pos()[axis],r);
		};
		~Box1D() {/* */}
	};

	template<int axis>
	class Box2D : public DistributionBase {
		/* Uniform distributions */
		Uniform uniform1,uniform2;
		/* Static constructor functions */
		static DistributionBase* xAxisConstructor(const Definition* definition) {
			return new Box2D<xaxis>(definition);
		}
		static DistributionBase* yAxisConstructor(const Definition* definition) {
			return new Box2D<yaxis>(definition);
		}
		static DistributionBase* zAxisConstructor(const Definition* definition) {
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
			return "box2d-" + getPlaneName<axis>();
		}
	public:
		Box2D() {/* */}
		Box2D(const Definition* definition) : DistributionBase(definition) {
			std::vector<double> coeffs = definition->getCoeffs();
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

	class Box3D : public DistributionBase {
		Uniform uniformx;
		Uniform uniformy;
		Uniform uniformz;
		static DistributionBase* Box3DConstructor(const Definition* definition) {
			return new Box3D(definition);
		}
		Constructor constructor() const {
			return Box3DConstructor;
		}
		/* Name of the distribution */
		std::string getName() const {
			return "box3d";
		}
	public:
		Box3D() {/* */}
		Box3D(const Definition* definition) : DistributionBase(definition) {
			std::vector<double> coeffs = definition->getCoeffs();
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

	class Isotropic : public DistributionBase {
		static DistributionBase* IsotropicConstructor(const Definition* definition) {
			return new Isotropic(definition);
		}
		Constructor constructor() const {
			return IsotropicConstructor;
		}
		/* Name of the distribution */
		std::string getName() const {
			return "isotropic";
		}
	public:
		Isotropic() {/* */}
		Isotropic(const Definition* definition) : DistributionBase(definition) {/* */}
		virtual void operator() (Particle& particle, Random& r) const {
			isotropicDirection(particle.dir(),r);
		};
		~Isotropic() {/* */}
	};

	class DistributionFactory {

		/* Static instance of the factory */
		static DistributionFactory factory;

		/* Map of surfaces types and constructors */
		std::map<std::string, DistributionBase::Constructor> constructor_table;

		/* Prevent construction or copy */
		DistributionFactory();
		DistributionFactory& operator= (const DistributionFactory& other);
		DistributionFactory(const DistributionFactory&);
		virtual ~DistributionFactory() {/* */}

	public:

		/* Access the factory, reference to the static singleton */
		static DistributionFactory& access() {return factory;}

		/* Register a new surface */
		void registerDistribution(const DistributionBase& surface);

		/* Create a new surface */
		DistributionBase* createDistribution(const DistributionBase::Definition* definition) const;

	};

} /* namespace Helios */
#endif /* DISTRIBUTION_HPP_ */
