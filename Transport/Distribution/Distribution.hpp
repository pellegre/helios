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
#include "../SourceDefinition.hpp"
#include "../Particle.hpp"

namespace Helios {

	/* Base class for distribution */
	class DistributionBase {

	public:
		/* Base class to define a distribution */
		class Definition : public SourceDefinition {
			/* Type of distribution */
			std::string type;
			/* Distribution ID on this problem */
			DistributionId distid;
		public:
			Definition(const std::string& type, const DistributionId& distid) :
				SourceDefinition(SourceDefinition::DIST), type(type), distid(distid) {/* */}
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

	/* Base class for distribution */
	class Distribution : public DistributionBase {

	public:
		/* Base class to define a distribution */
		class Definition : public DistributionBase::Definition {
			/* Coefficients for each child */
			std::vector<double> coeffs;
		public:
			Definition(const std::string& type, const DistributionId& distid, const std::vector<double>& coeffs) :
				DistributionBase::Definition(type,distid) , coeffs(coeffs) {/* */}
			std::vector<double> getCoeffs() const {
				return coeffs;
			}
			virtual ~Definition() {/* */}
		};

		/* Constructor from definition */
		Distribution(const DistributionBase::Definition* definition) : DistributionBase(definition) {/* */};
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

	/* Base class for distribution */
	class DistributionCustom : public DistributionBase {

	public:
		/* Base class to define a distribution */
		class Definition : public DistributionBase::Definition {
			/* Samplers IDs */
			std::vector<DistributionId> samplersIds;
			/* Weights of each sampler */
			std::vector<double> weights;

			/* Samplers */
			std::vector<DistributionBase*> samplers;

		public:
			Definition(const std::string& type, const DistributionId& distid,
					   const std::vector<DistributionId>& samplersIds, const std::vector<double>& weights) :
				DistributionBase::Definition(type,distid) , samplersIds(samplersIds) , weights(weights) {
				/* Check the weight input */
				if(this->weights.size() == 0) {
					this->weights.resize(this->samplersIds.size());
					/* Equal probability for all samplers */
					double prob = 1/(double)this->samplersIds.size();
					for(size_t i = 0 ; i < this->samplersIds.size() ; ++i)
						this->weights[i] = prob;
				}
			}

			virtual ~Definition() {/* */}

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

		/* Constructor from definition */
		DistributionCustom(const DistributionBase::Definition* definition) : DistributionBase(definition) {
			const DistributionCustom::Definition* distDefinition = static_cast<const DistributionCustom::Definition*>(definition);
			/* Weights of each sampler */
			std::vector<double> weights = distDefinition->getWeights();
			/* Samplers */
			std::vector<DistributionBase*> samplers = distDefinition->getDistributions();
			/* Create sampler */
			distribution_sampler = new Sampler<DistributionBase*>(samplers,weights);
		};

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
		static DistributionBase* CustomConstructor(const DistributionBase::Definition* definition) {
			return new DistributionCustom(definition);
		}
		Constructor constructor() const {
			return CustomConstructor;
		}
		/* Sampler of ParticleSampler(s) */
		Sampler<DistributionBase*>* distribution_sampler;
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
