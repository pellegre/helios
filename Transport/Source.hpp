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
#include "SourceDefinition.hpp"
#include "ParticleSource.hpp"

namespace Helios {

	/*
	 * The source contains all the distributions, samplers and particle-sources on the problem.
	 * When the source is eliminated, all this resources are deallocated. This class is the equivalent
	 * to the Geometry class for cells and surfaces.
	 */
	class Source {

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

		/* Setup the source from the base definitions */
		void setupSource(std::vector<SourceDefinition*>& definitions);

		/* Once the definitions are dispatched, setup source with this function */
		void setupSource(std::vector<DistributionBase::Definition*>& distDefinition,
				         std::vector<ParticleSampler::Definition*>& samplerDefinition,
				         std::vector<ParticleSource::Definition*>& sourceDefinition);

		/* Maps of internal IDs with user defined IDs */
		std::map<DistributionId,InternalDistributionId> distribution_map;
		std::map<SamplerId,InternalSamplerId> sampler_map;

	public:
		/* Construction from definitions */
		Source(std::vector<SourceDefinition*>& definitions) {setupSource(definitions);}

		/* Sample a particle */
		Particle sample(Random& r) const {
			Particle particle;
			ParticleSource* sampler = source_sampler->sample(0,r.uniform());
			sampler->sample(particle,r);
			return particle;
		}

		~Source();
	};

} /* namespace Helios */

#endif /* SOURCE_HPP_ */
