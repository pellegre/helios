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

#include "ParticleSource.hpp"
#include "Source.hpp"
#include "../Environment/McEnvironment.hpp"

using namespace std;

namespace Helios {


ParticleSource::ParticleSource(const ParticleSourceObject* definition, const Source* source) : strength(definition->getStrength()) {
	/* Get distributions */
	vector<SamplerId> sampler_ids = definition->getSamplersIds();

	/* Get instances of the samplers IDs */
	vector<ParticleSampler*> samplers;
	for(vector<SamplerId>::iterator it = sampler_ids.begin() ; it != sampler_ids.end() ; ++it)
		samplers.push_back(source->getObject<ParticleSampler>((*it))[0]);

	/* Get weight of each sampler */
	std::vector<double> weights = definition->getWeights();
	/* Create a sampler */
	source_sampler = new Sampler<ParticleSampler*>(samplers,weights);
}

ParticleSampler::ParticleSampler(const ParticleSamplerObject* definition, const Source* source) :
		user_id(definition->getSamplerid()), position(definition->getPosition()),
		direction(definition->getDirection()), energy(1.0), weight(1.0), state(Particle::ALIVE) {

	/* Get distributions */
	vector<DistributionId> distribution_ids = definition->getDistributionIds();

	for(vector<DistributionId>::iterator it = distribution_ids.begin() ; it != distribution_ids.end() ; ++it)
		distributions.push_back(source->getObject<DistributionBase>((*it))[0]);
}

ParticleSourceObject::ParticleSourceObject(const std::vector<SamplerId>& samplersIds, const std::vector<double>& weights, const double& strength) :
	SourceObject(ParticleSource::name()), samplersIds(samplersIds), weights(weights), strength(strength) {
	/* Check the weight input */
	if(this->weights.size() == 0) {
		this->weights.resize(this->samplersIds.size());
		/* Equal probability for all samplers */
		double prob = 1/(double)this->samplersIds.size();
		for(size_t i = 0 ; i < this->samplersIds.size() ; ++i)
			this->weights[i] = prob;
	}
}
} /* namespace Helios */
