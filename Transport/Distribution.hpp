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
		public:
			Definition(const std::string& type, const DistributionId& distid) : type(type), distid(distid) {/* */}

			DistributionId getUserId() const {
				return distid;
			}
			std::string getType() const {
				return type;
			}
			virtual ~Definition() {/* */}
		};

		/* Exception */
		class BadDistributionCreation : public std::exception {
			std::string reason;
		public:
			BadDistributionCreation(const DistributionId& distid, const std::string& msg) {
				reason = "Cannot create cell " + toString(distid) + " : " + msg;
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

		/* Sample a real value */
		virtual void operator() (double& value, Random& r) const {/* */};
		/* Sample a coordinate */
		virtual void operator() (Coordinate& position, Random& r) const {/* */};

		virtual ~DistributionBase() {/* */};

		DistributionId getDistid() const {
			return distid;
		}

		virtual std::string getName() const  = 0;

		protected:
			/* Get constructor */
			virtual Constructor constructor() const = 0;
		private:
			/* Distribution ID defined by the user */
			DistributionId distid;
	};

	template<int axis>
	class Uniform : public DistributionBase {
		/* Points defining the box */
		double umax,umin;
	public:
		Uniform() {/* */}
		virtual void operator() (Coordinate& position, Random& r) const {
			position[axis] += (umax - umin)*r.uniform() + umin;
		};
		~Uniform() {/* */}
	};

	template<int axis>
	class Box1D : public DistributionBase {
		Uniform<axis> uniform;
	public:
		Box1D() {/* */}
		virtual void operator() (Coordinate& position, Random& r) const {
			uniform(position,r);
		};
		~Box1D() {/* */}
	};

	template<int axis1, int axis2>
	class Box2D : public DistributionBase {
		Uniform<axis1> uniform1;
		Uniform<axis2> uniform2;
	public:
		Box2D() {/* */}
		virtual void operator() (Coordinate& position, Random& r) const {
			uniform1(position,r);
			uniform2(position,r);
		};
		~Box2D() {/* */}
	};

	template<int axis1, int axis2, int axis3>
	class Box3D : public DistributionBase {
		Uniform<axis1> uniform1;
		Uniform<axis2> uniform2;
		Uniform<axis2> uniform3;
	public:
		Box3D() {/* */}
		virtual void operator() (Coordinate& position, Random& r) const {
			uniform1(position,r);
			uniform2(position,r);
			uniform3(position,r);
		};
		~Box3D() {/* */}
	};

	class Isotropic : public DistributionBase {
	public:
		Isotropic() {/* */}
		virtual void operator() (Direction& direction, Random& r) const {
			isotropicDirection(direction,r);
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
