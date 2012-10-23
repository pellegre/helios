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

#include "Source.hpp"

using namespace std;

namespace Helios {

McModule* SourceFactory::create(const std::vector<McObject*>& objects) const {
	return new Source(objects,getEnvironment());
}

template<class T>
static void pushObject(McObject* src, vector<T*>& definition) {
	definition.push_back(dynamic_cast<T*>(src));
}

Source::Source(const vector<McObject*>& definitions, const McEnvironment* environment) : McModule(name(),environment) {
	/* Containers of each source object */
	vector<DistributionBaseObject*> distObject;
	vector<ParticleSamplerObject*> samplerObject;
	vector<ParticleSourceObject*> sourceObject;

	/* Dispatch each definition to the corresponding container */
	vector<McObject*>::const_iterator it_def = definitions.begin();

	/* Get the type and put the object on the correct container */
	for(; it_def != definitions.end() ; ++it_def) {
		string type = (*it_def)->getObjectName();
		if (type == DistributionBase::name())
			pushObject(*it_def,distObject);
		else if (type == ParticleSampler::name())
			pushObject(*it_def,samplerObject);
		else if (type == ParticleSource::name())
			pushObject(*it_def,sourceObject);
	}

	/* Create distributions */
	distributions = distribution_factory.createDistributions(distObject);

	/* Update the distribution map */
	for(size_t i = 0; i < distributions.size() ; ++i)
		distribution_map[distributions[i]->getUserId()] = distributions[i];

	/* Create samplers */
	vector<ParticleSamplerObject*>::const_iterator itSampler = samplerObject.begin();
	for(; itSampler != samplerObject.end() ; ++itSampler) {
		ParticleSampler* sampler = new ParticleSampler((*itSampler),this);
		sampler_map[(*itSampler)->getSamplerid()] = sampler;
		particle_samplers.push_back(sampler);
	}

	if(sourceObject.size() == 0)
		throw(ParticleSource::BadSourceCreation("There isn't sources definitions available"));

	/* Finally, create the source with each sampler */
	vector<double> strengths;
	vector<ParticleSourceObject*>::const_iterator itSource = sourceObject.begin();
	for(; itSource != sourceObject.end() ; ++itSource) {
		ParticleSource* source = new ParticleSource((*itSource),this);
		sources.push_back(source);
		strengths.push_back(source->getStrength());
	}

	/* Once we got all the sources, we should create the sampler */
	source_sampler = new Sampler<ParticleSource*>(sources,strengths);
}

void Source::print(std::ostream& out) const {
	/* Get distributions */
	vector<ParticleSource*> samplers = source_sampler->getReactions();
	/* Reaction matrix */
	const double* reaction_matrix = source_sampler->getReactionMatrix();
	/* Print each distributions */
	size_t i = 0;
	for( ; i < samplers.size() - 1 ; ++i)
		out << " ( cdf = " << reaction_matrix[i] << " ) " << endl << *samplers[i];
	/* Last one... */
	out << " ( cdf =  1.0 ) "<< endl << *samplers[i];
}

template<>
const std::map<UserId,DistributionBase*>& Source::getObjectMap<DistributionBase>() const {
	return distribution_map;
}

template<>
const std::map<UserId,ParticleSampler*>& Source::getObjectMap<ParticleSampler>() const {
	return sampler_map;
}

Source::~Source() {
	purgePointers(distributions);
	purgePointers(particle_samplers);
	purgePointers(sources);
	delete source_sampler;
}
} /* namespace Helios */
