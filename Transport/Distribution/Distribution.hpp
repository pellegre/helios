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

#include "../../Common/Common.hpp"
#include "../../Common/Sampler.hpp"
#include "../SourceObject.hpp"
#include "../Particle.hpp"

namespace Helios {
	class DistributionBaseObject;

	/* Base class for distribution */
	class DistributionBase {

	public:

		static std::string name() {return "distribution";}

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
		typedef DistributionBase(*(*Constructor)(const DistributionBaseObject*));
		/* Friendly factory */
		friend class DistributionFactory;

		/* Constructor from definition */
		DistributionBase(const DistributionBaseObject* definition);
		/* Only used by the factory */
		DistributionBase() : user_id() {/* */};

		/* Sample a coordinate */
		virtual void operator() (Particle& particle, Random& r) const = 0;

		virtual ~DistributionBase() {/* */};

		DistributionId getUserId() const {
			return user_id;
		}

	protected:
		/* Get name of the distribution */
		virtual std::string getName() const  = 0;
		/* Get constructor */
		virtual Constructor constructor() const = 0;
	private:
		/* Distribution ID defined by the user */
		DistributionId user_id;
	};

	/* Base class for distribution */
	class Distribution : public DistributionBase {

	public:

		/* Constructor from definition */
		Distribution(const DistributionBaseObject* definition) : DistributionBase(definition) {/* */};
		/* Only used by the factory */
		Distribution() {/* */};

		/* Sample a coordinate */
		virtual void operator() (Particle& particle, Random& r) const = 0;

		virtual ~Distribution() {/* */};

	protected:
		/* Get name of the distribution */
		virtual std::string getName() const  = 0;
		/* Get constructor */
		virtual Constructor constructor() const = 0;
	};

	/* Base class for distribution */
	class DistributionCustom : public DistributionBase {

	public:

		/* Constructor from definition */
		DistributionCustom(const DistributionBaseObject* definition);

		/* Only used by the factory */
		DistributionCustom() : DistributionBase() , distribution_sampler(0) {};

		/* Sample a coordinate */
		virtual void operator() (Particle& particle, Random& r) const {
			DistributionBase* dist = distribution_sampler->sample(0,r.uniform());
			/* Apply distribution */
			(*dist)(particle,r);
		};

		virtual ~DistributionCustom() {delete distribution_sampler;};

	protected:
		/* Get name of the distribution */
		virtual std::string getName() const  {return "custom";};
		/* Get constructor */
		static DistributionBase* CustomConstructor(const DistributionBaseObject* definition) {
			return new DistributionCustom(definition);
		}
		Constructor constructor() const {
			return CustomConstructor;
		}
		/* Sampler of ParticleSampler(s) */
		Sampler<DistributionBase*>* distribution_sampler;
	};

	/* ------- Objects for distributions */

	/* Base class to define a distribution */
	class DistributionBaseObject : public SourceObject {
		/* Type of distribution */
		std::string type;
		/* Distribution ID on this problem */
		DistributionId distid;
	public:
		DistributionBaseObject(const std::string& type, const DistributionId& distid) :
			SourceObject(DistributionBase::name()), type(type), distid(distid) {/* */}
		DistributionId getUserId() const {
			return distid;
		}
		std::string getType() const {
			return type;
		}
		virtual ~DistributionBaseObject() {/* */}
	};

	/* Base class to define a distribution */
	class DistributionObject : public DistributionBaseObject {
		/* Coefficients for each child */
		std::vector<double> coeffs;
	public:
		DistributionObject(const std::string& type, const DistributionId& distid, const std::vector<double>& coeffs) :
			DistributionBaseObject(type,distid) , coeffs(coeffs) {/* */}
		std::vector<double> getCoeffs() const {
			return coeffs;
		}
		virtual ~DistributionObject() {/* */}
	};

	/* Base class to define a distribution */
	class DistributionCustomObject : public DistributionBaseObject {
		/* Samplers IDs */
		std::vector<DistributionId> samplersIds;
		/* Weights of each sampler */
		std::vector<double> weights;

		/* Samplers */
		std::vector<DistributionBase*> samplers;

	public:
		DistributionCustomObject(const std::string& type, const DistributionId& distid,
				   const std::vector<DistributionId>& samplersIds, const std::vector<double>& weights);

		virtual ~DistributionCustomObject() {/* */}

		std::vector<DistributionBase*> getDistributions() const {
			return samplers;
		}

		void setDistributions(std::vector<DistributionBase*> samplers) {
			this->samplers = samplers;
		}

		std::vector<DistributionId> getDistributionIds() const {
			return samplersIds;
		}

		std::vector<double> getWeights() const {
			return weights;
		}

	};

	class DistributionFactory {
		/* Map of surfaces types and constructors */
		std::map<std::string, DistributionBase::Constructor> constructor_table;
		/* Create a Distribution */
		DistributionBase* createDistribution(const DistributionBaseObject* definition) const;
	public:
		/* Prevent construction or copy */
		DistributionFactory();
		/* Register a new surface */
		void registerDistribution(const DistributionBase& distribution);
		/* Create a new surface */
		std::vector<DistributionBase*> createDistributions(const std::vector<DistributionBaseObject*>& definitions) const;
		virtual ~DistributionFactory() {/* */}
	};

} /* namespace Helios */
#endif /* DISTRIBUTION_HPP_ */
