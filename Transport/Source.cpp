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

template<class T>
static void pushDefinition(SourceDefinition* src, vector<T*>& definition) {
	definition.push_back(dynamic_cast<T*>(src));
}

void Source::setupSource(vector<SourceDefinition*>& definitions) {
	vector<DistributionBase::Definition*> distDefinition;
	vector<ParticleSampler::Definition*> samplerDefinition;
	vector<ParticleSource::Definition*> sourceDefinition;
	/* Dispatch each definition to the corresponding container */
	vector<SourceDefinition*>::const_iterator it_def = definitions.begin();

	for(; it_def != definitions.end() ; ++it_def) {
		switch((*it_def)->getType()) {
		case SourceDefinition::DIST:
			pushDefinition(*it_def,distDefinition);
			break;
		case SourceDefinition::SAMPLER:
			pushDefinition(*it_def,samplerDefinition);
			break;
		case SourceDefinition::SOURCE:
			pushDefinition(*it_def,sourceDefinition);
			break;
		}
	}

	setupSource(distDefinition,samplerDefinition,sourceDefinition);
	definitions.clear();
}

void Source::setupSource(vector<DistributionBase::Definition*>& distDefinition,
		                 vector<ParticleSampler::Definition*>& samplerDefinition,
		                 vector<ParticleSource::Definition*>& sourceDefinition) {

	/* Container of custom distributions */
	vector<DistributionCustom::Definition*> customDefinition;

	/* Create the distributions */
	vector<DistributionBase::Definition*>::const_iterator itDist = distDefinition.begin();
	for(; itDist != distDefinition.end() ; ++itDist) {
		DistributionId id = (*itDist)->getUserId();
		if(distribution_map.find(id) != distribution_map.end())
			throw(DistributionBase::BadDistributionCreation(id,"Duplicated id"));
		if((*itDist)->getType() != "custom") {
			/* Update distribution map */
			distribution_map[id] = distributions.size();
			/* Create the distribution */
			DistributionBase* distPtr = DistributionFactory::access().createDistribution((*itDist));
			/* Push it into the container */
			distributions.push_back(distPtr);
		} else
			customDefinition.push_back(static_cast<DistributionCustom::Definition*>(*itDist));
	}

	/* Create custom definitions */
	vector<DistributionCustom::Definition*>::const_iterator itCustom = customDefinition.begin();
	for(; itCustom != customDefinition.end() ; ++itCustom) {
		/* Get distributions */
		vector<DistributionId> distIds = (*itCustom)->getDistributionIds();
		vector<DistributionBase*> distPtrs;
		for(vector<DistributionId>::iterator it = distIds.begin() ; it != distIds.end() ; ++it) {
			map<DistributionId,InternalDistributionId>::iterator itId = distribution_map.find((*it));
			if(itId == distribution_map.end())
				throw(DistributionBase::BadDistributionCreation((*itCustom)->getUserId(),
					  "Distribution id " + toString((*it)) + " does not exist"));
			else
				distPtrs.push_back(distributions[(*itId).second]);
		}
		/* Put the distribution container into the definition */
		(*itCustom)->setDistributions(distPtrs);
		/* Update distribution map */
		distribution_map[(*itCustom)->getUserId()] = distributions.size();
		/* Create the distribution */
		DistributionBase* distPtr = DistributionFactory::access().createDistribution((*itCustom));
		/* Push it into the container */
		distributions.push_back(distPtr);
	}

	/* Create samplers */
	vector<ParticleSampler::Definition*>::const_iterator itSampler = samplerDefinition.begin();
	for(; itSampler != samplerDefinition.end() ; ++itSampler) {
		/* Get distributions */
		vector<DistributionId> distIds = (*itSampler)->getDistributionIds();
		vector<DistributionBase*> distPtrs;
		for(vector<DistributionId>::iterator it = distIds.begin() ; it != distIds.end() ; ++it) {
			map<DistributionId,InternalDistributionId>::iterator itId = distribution_map.find((*it));
			if(itId == distribution_map.end())
				throw(ParticleSampler::BadSamplerCreation((*itSampler)->getSamplerid(),
					  "Distribution id <" + toString((*it)) + "> does not exist"));
			else
				distPtrs.push_back(distributions[(*itId).second]);
		}
		/* Put the distribution container into the definition */
		(*itSampler)->setDistributions(distPtrs);
		ParticleSampler* sampler = new ParticleSampler((*itSampler));
		sampler_map[(*itSampler)->getSamplerid()] = particle_samplers.size();
		particle_samplers.push_back(sampler);
	}

	if(sourceDefinition.size() == 0)
		throw(ParticleSource::BadSourceCreation("You didn't put any source on the <sources> node in the input file"));

	/* Finally, create the source with each samplers */
	vector<double> strengths;
	vector<ParticleSource::Definition*>::const_iterator itSource = sourceDefinition.begin();
	for(; itSource != sourceDefinition.end() ; ++itSource) {
		/* Get distributions */
		vector<SamplerId> samplerIds = (*itSource)->getSamplersIds();
		vector<ParticleSampler*> samplerPtrs;
		for(vector<SamplerId>::iterator it = samplerIds.begin() ; it != samplerIds.end() ; ++it) {
			map<SamplerId,InternalSamplerId>::iterator itId = sampler_map.find((*it));
			if(itId == sampler_map.end())
				throw(ParticleSource::BadSourceCreation("Sampler id <" + toString((*it)) + "> does not exist"));
			else
				samplerPtrs.push_back(particle_samplers[(*itId).second]);
		}
		/* Put the distribution container into the definition */
		(*itSource)->setSamplers(samplerPtrs);
		ParticleSource* source = new ParticleSource((*itSource));
		sources.push_back(source);
		strengths.push_back(source->getStrength());
	}

	purgePointers(distDefinition);
	purgePointers(samplerDefinition);
	purgePointers(sourceDefinition);

	/* Once we got all the sources, we should create the sampler */
	source_sampler = new Sampler<ParticleSource*>(sources,strengths);
}

Source::~Source() {
	purgePointers(distributions);
	purgePointers(particle_samplers);
	purgePointers(sources);
	delete source_sampler;
}
} /* namespace Helios */
