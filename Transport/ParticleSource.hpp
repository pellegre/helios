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

#ifndef PARTICLESOURCE_HPP_
#define PARTICLESOURCE_HPP_

#include <vector>

#include "../Common/Common.hpp"
#include "../Common/Sampler.hpp"
#include "SourceObject.hpp"
#include "Distribution/Distribution.hpp"

namespace Helios {

	class ParticleSamplerObject;
	class ParticleSourceObject;
	class Source;

	/* Sampling a particle (position, energy and angle) */
	class ParticleSampler {
		/* ID defined by the user for this sampler */
		SamplerId user_id;
		/* Reference position of the sampler */
		Coordinate position;
		/* Reference direction of the sampler */
		Direction direction;
		/* Energy of the initial particle (1 MeV by default) */
		double energy;
		/* Initial weight */
		double weight;
		/* Initial state of the particle */
		Particle::State state;

		/* Sampling different stuff on phase space */
		std::vector<DistributionBase*> distributions;

		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const ParticleSampler& q);
	public:
		/* Name of this object */
		static std::string name() {return "sampler";}

		/* Exception */
		class BadSamplerCreation : public std::exception {
			std::string reason;
		public:
			BadSamplerCreation(const SamplerId& distid, const std::string& msg) {
				reason = "Cannot create sampler " + toString(distid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSamplerCreation() throw() {/* */};
		};

		ParticleSampler(const ParticleSamplerObject* definition, const Source* source);

		SamplerId getUserId() const {
			return user_id;
		}

		/* Sample particle */
		virtual void operator() (Particle& particle,Random& r) const {
			/* Set phase space coordinates of this sampler */
			particle.pos() = position;
			particle.dir() = direction;
			particle.evs() = energy;
			particle.eix() = 0;
			particle.wgt() = weight;
			particle.sta() = state;
			/* Apply distributions (if any) */
			for(std::vector<DistributionBase*>::const_iterator it = distributions.begin() ; it != distributions.end() ; ++it)
				(*(*it))(particle,r);
		}

		virtual ~ParticleSampler(){/* */};
	};

	class ParticleSource {

	public:
		friend std::ostream& operator<<(std::ostream& out, const ParticleSource& q);

		/* Name of this object */
		static std::string name() {return "source";}

		/* Exception */
		class BadSourceCreation : public std::exception {
			std::string reason;
		public:
			BadSourceCreation(const std::string& msg) {
				reason = "Cannot create source : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSourceCreation() throw() {/* */};
		};

		/* Create the source */
		ParticleSource(const ParticleSourceObject* definition, const Source* source);

		/* Sample a particle */
		Particle sample(Random& r) const {
			Particle particle;
			ParticleSampler* sampler = source_sampler->sample(0,r.uniform());
			(*sampler)(particle,r);
			return particle;
		}

		/* Sample a particle (override) */
		void sample(Particle& particle, Random& r) const {
			ParticleSampler* sampler = source_sampler->sample(0,r.uniform());
			(*sampler)(particle,r);
		}

		/* Get strength */
		double getStrength() const {
			return strength;
		}

		virtual ~ParticleSource() {delete source_sampler;};

	private:

		/* Sampler of ParticleSampler(s) */
		Sampler<ParticleSampler*>* source_sampler;
		/* Strength of this source */
		double strength;
	};

	/* Printers */
	std::ostream& operator<<(std::ostream& out, const ParticleSampler& q);
	std::ostream& operator<<(std::ostream& out, const ParticleSource& q);

	/* ------- Objects for samplers and sources */

	class ParticleSamplerObject : public SourceObject {
		/* ID defined by the user for this sampler */
		SamplerId sampler_id;
		/* Reference position of the sampler */
		Coordinate position;
		/* Reference direction of the sampler */
		Direction direction;
		/* Samplers IDs */
		std::vector<DistributionId> distribution_ids;

		friend class ParticleSampler;
	public:
		ParticleSamplerObject(const SamplerId& sampler_id, const Coordinate& position,
				   const Direction& direction, const std::vector<DistributionId>& distribution_ids) :
				   SourceObject(ParticleSampler::name()), sampler_id(sampler_id),
				   position(position), direction(direction), distribution_ids(distribution_ids) {/* */}

		Direction getDirection() const {
			return direction;
		}

		Coordinate getPosition() const {
			return position;
		}

		SamplerId getSamplerid() const {
			return sampler_id;
		}

		std::vector<DistributionId> getDistributionIds() const {
			return distribution_ids;
		}

		~ParticleSamplerObject() {/* */}
	};

	class ParticleSourceObject : public SourceObject {
		/* Samplers IDs */
		std::vector<SamplerId> samplers_ids;
		/* Weights of each sampler */
		std::vector<double> weights;
		/* Strength of the source on the problem */
		double strength;

		/* Samplers */
		std::vector<ParticleSampler*> samplers;
		friend class ParticleSource;
	public:
		ParticleSourceObject(const std::vector<SamplerId>& samplers_ids, const std::vector<double>& weights,
				const double& strength);

		std::vector<SamplerId> getSamplersIds() const {
			return samplers_ids;
		}

		std::vector<double> getWeights() const {
			return weights;
		}
		double getStrength() const {
			return strength;
		}

		~ParticleSourceObject() {/* */}
	};
} /* namespace Helios */
#endif /* PARTICLESOURCE_HPP_ */
