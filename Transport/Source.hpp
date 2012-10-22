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

#ifndef SOURCE_HPP_
#define SOURCE_HPP_

#include <map>
#include <vector>
#include "../Common/Sampler.hpp"
#include "SourceObject.hpp"
#include "ParticleSource.hpp"

namespace Helios {

	/*
	 * The source contains all the distributions, samplers and particle-sources on the problem.
	 * When the source is eliminated, all this resources are deallocated. This class is the equivalent
	 * to the Geometry class for cells and surfaces.
	 */
	class Source : public McModule {

		/* Distribution factory */
		DistributionFactory distribution_factory;

		/* Distributions */
		std::vector<DistributionBase*> distributions;
		/*
		 * Particle samplers, this are conformed by a combination of
		 * different distributions (and produce a particle in phase space)
		 */
		std::vector<ParticleSampler*> particle_samplers;
		/* Particles sources, these are a combination of Particle samplers */
		std::vector<ParticleSource*> sources;

		/* A sampler of sources */
		Sampler<ParticleSource*>* source_sampler;

		/* Once the definitions are dispatched, setup source with this function */
		void setupSource(std::vector<DistributionBaseObject*>& distObject,
				         std::vector<ParticleSamplerObject*>& samplerObject,
				         std::vector<ParticleSourceObject*>& sourceObject);

		/* Maps of internal IDs with user defined IDs */
		std::map<DistributionId,InternalDistributionId> distribution_map;
		std::map<SamplerId,InternalSamplerId> sampler_map;

	public:
		/* Name of this module */
		static std::string name() {return "sources";}

		/* Construction from definitions */
		Source(const std::vector<McObject*>& definitions, const McEnvironment* environment);

		/* Sample a particle */
		Particle sample(Random& r) const {
			Particle particle;
			ParticleSource* sampler = source_sampler->sample(0,r.uniform());
			sampler->sample(particle,r);
			return particle;
		}

		~Source();
	};

	class McEnvironment;

	/* Material Factory */
	class SourceFactory : public ModuleFactory {
	public:
		/* Prevent construction or copy */
		SourceFactory(McEnvironment* environment) : ModuleFactory(Source::name(),environment) {/* */};
		/* Create a new material */
		McModule* create(const std::vector<McObject*>& objects) const;
		virtual ~SourceFactory() {/* */}
	};

} /* namespace Helios */

#endif /* SOURCE_HPP_ */
