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

#include <algorithm>

#include "ParticleSource.hpp"
#include "Source.hpp"
#include "../Environment/McEnvironment.hpp"

using namespace std;

namespace Helios {

ParticleSampler::ParticleSampler(const ParticleSamplerObject* definition, const Source* source) :
		user_id(definition->getSamplerid()), position(definition->getPosition()),
		direction(definition->getDirection()), energy(1.0), weight(1.0), state(Particle::ALIVE) {

	/* Get distributions */
	vector<DistributionId> distribution_ids = definition->getDistributionIds();

	for(vector<DistributionId>::iterator it = distribution_ids.begin() ; it != distribution_ids.end() ; ++it)
		distributions.push_back(source->getObject<DistributionBase>((*it))[0]);
}

static inline bool isPositionDistribution(const DistributionBase* distribution) {
	/* Create particle */
	Particle particle;
	particle.pos() = Coordinate(0,0,0);
	/* Particle to apply the distribution */
	Particle sampled_particle(particle);
	/* Create random number */
	Random random(1);
	/* Apply distribution */
	(*distribution)(sampled_particle,random);
	/* Return comparison */
	return !compareTinyVector(particle.pos(),sampled_particle.pos());
}

ParticleCellSampler::ParticleCellSampler(const ParticleSamplerObject* definition, const Source* source)
         : ParticleSampler(definition,source) {

	/* Get cells */
	try {
		cells = source->getEnvironment()->getObject<Geometry,Cell>(definition->getCellId());
	} catch (exception& error) {
		throw(BadSamplerCreation(getUserId(),error.what()));
	}
	/*
	 * We have a problem here... From all the distributions given by the user, we should
	 * find out which ones are related to the position variable in phase space. And reject
	 * or accept the particles based on the cell specified ONLY sampling the distributions that
	 * change the position of the particle.
	 */

	for(vector<DistributionBase*>::const_iterator it = distributions.begin() ; it != distributions.end() ; ++it) {
		/* Find positions distributions */
		if(isPositionDistribution(*it)) {
			/* Push to the container */
			pos_distributions.push_back((*it));
			cout << "pos = " << *(*it) << endl;
		}
	}

	/* Remove those from the original container */
	vector<DistributionBase*>::iterator it = remove_if(distributions.begin(),distributions.end(),isPositionDistribution);
	distributions.resize(it - distributions.begin());

	for(it = distributions.begin() ; it != distributions.end() ; ++it)
		cout << "no-pos = " << *(*it) << endl;
}

/* Sample particle (and check cell) */
void ParticleCellSampler::operator() (Particle& particle,Random& r) const {
	/* Number of samples */
	int nsamples = 0;
	/* Flag if is inside the cell */
	bool inside = false;
	/* First get the position (rejecting point outside the cell) */
	while(!inside) {
		/* Initial position for sampling */
		particle.pos() = position;
		/* Apply position distributions */
		for(vector<DistributionBase*>::const_iterator it = pos_distributions.begin() ; it != pos_distributions.end() ; ++it) {
			(*(*it))(particle,r);
			cout << particle.pos() << endl;
		}
		/* Check if we are inside the cell */
		for(vector<Cell*>::const_iterator it = cells.begin() ; it != cells.end() ; ++it) {
			if((*it)->isInside(particle.pos())) {
				inside = true;
				break;
			}
		}
		/* Count sample */
		nsamples++;
		if(nsamples >= ParticleSampler::max_samples)
			throw(GeneralError("Sampler efficiency too low on sampler " + getUserId() +
					". Please, reconsider the source definition because this is not a fair game"));
	}
	/* Once the position is set, set phase space coordinates of this sampler */
	particle.dir() = direction;
	particle.evs() = energy;
	particle.eix() = 0;
	particle.wgt() = weight;
	particle.sta() = state;
	/* Apply distributions (if any) */
	for(vector<DistributionBase*>::const_iterator it = distributions.begin() ; it != distributions.end() ; ++it)
		(*(*it))(particle,r);
}

std::ostream& operator<<(std::ostream& out, const ParticleSampler& q) {
	out << "sampler = " << q.getUserId()
		<< " ; position = " << q.position
		<< " ; direction = " << q.direction
		<< " ; energy = " << q.energy
		<< " ; weight = " << q.weight
		<< endl;
	for(std::vector<DistributionBase*>::const_iterator it = q.distributions.begin() ; it != q.distributions.end() ; ++it)
		out << Log::ident(2) << " * " << *(*it) << endl;
	return out;
}

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

std::ostream& operator<<(std::ostream& out, const ParticleSource& q) {
	/* Get distributions */
	vector<ParticleSampler*> samplers = q.source_sampler->getReactions();
	/* Reaction matrix */
	const double* reaction_matrix = q.source_sampler->getReactionMatrix();
	/* Print each distributions */
	size_t i = 0;
	for( ; i < samplers.size() - 1 ; ++i)
		out << Log::ident(1) << " ( cdf = " << fixed << reaction_matrix[i] << " ) " << *samplers[i];
	/* Last one... */
	out << Log::ident(1) << " ( cdf = " << 1.0 << " ) " << *samplers[i];
	return out;
}

ParticleSourceObject::ParticleSourceObject(const std::vector<SamplerId>& samplers_ids, const std::vector<double>& weights, const double& strength) :
	SourceObject(ParticleSource::name()), samplers_ids(samplers_ids), weights(weights), strength(strength) {
	/* Check the weight input */
	if(this->weights.size() == 0) {
		this->weights.resize(this->samplers_ids.size());
		/* Equal probability for all samplers */
		double prob = 1/(double)this->samplers_ids.size();
		for(size_t i = 0 ; i < this->samplers_ids.size() ; ++i)
			this->weights[i] = prob;
	}
}

/* Create particle sampler */
ParticleSampler* SamplerFactory::create(const ParticleSamplerObject* definition, const Source* source) const {

	if(definition->getCellId() == "0")
		return new ParticleSampler(definition,source);
	else
		return new ParticleCellSampler(definition,source);
}

} /* namespace Helios */
