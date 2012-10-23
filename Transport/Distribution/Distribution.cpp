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

#include <set>

#include "Distribution.hpp"
#include "Spatial.hpp"
#include "Angular.hpp"

using namespace std;

namespace Helios {

DistributionFactory::DistributionFactory() {
	/* Distribution registering */
	registerDistribution(Box1D<xaxis>());
	registerDistribution(Box1D<yaxis>());
	registerDistribution(Box1D<zaxis>());
	registerDistribution(Box2D<xaxis>());
	registerDistribution(Box2D<yaxis>());
	registerDistribution(Box2D<zaxis>());
	registerDistribution(Box3D());
	registerDistribution(Cyl2D<xaxis>());
	registerDistribution(Cyl2D<yaxis>());
	registerDistribution(Cyl2D<zaxis>());
	registerDistribution(Isotropic());
}

DistributionBase* DistributionFactory::createDistribution(const DistributionBaseObject* definition) const {
	/* Create this particular distribution */
	map<string,DistributionBase::Constructor>::const_iterator it_type = constructor_table.find(definition->getType());
	if(it_type != constructor_table.end())
		return (*it_type).second(definition);
	else
		throw DistributionBase::BadDistributionCreation(definition->getUserId(),
				"Distribution type " + definition->getType() + " is not defined");
}

std::vector<DistributionBase*> DistributionFactory::createDistributions(const std::vector<DistributionBaseObject*>& dist_objects) const {

	/* Container of user distributions */
	std::vector<DistributionBase*> distributions;

	/* Container of custom distributions */
	vector<DistributionCustomObject*> custom_object;

	/* Map of user IDs with the distribution objects */
	map<DistributionId,DistributionBase*> distribution_map;

	/* Create the distributions */
	vector<DistributionBaseObject*>::const_iterator it_distribution = dist_objects.begin();
	for(; it_distribution != dist_objects.end() ; ++it_distribution) {
		DistributionId id = (*it_distribution)->getUserId();
		if(distribution_map.find(id) != distribution_map.end())
			throw(DistributionBase::BadDistributionCreation(id,"Duplicated id"));
		if((*it_distribution)->getType() != "custom") {
			/* Create the distribution */
			DistributionBase* dist_ptr = createDistribution((*it_distribution));
			/* Update distribution map */
			distribution_map[id] = dist_ptr;
			/* Push it into the container */
			distributions.push_back(dist_ptr);
		} else
			custom_object.push_back(static_cast<DistributionCustomObject*>(*it_distribution));
	}

	/* Create custom definitions */
	vector<DistributionCustomObject*>::const_iterator it_custom = custom_object.begin();
	for(; it_custom != custom_object.end() ; ++it_custom) {
		/* Get distributions IDs */
		vector<DistributionId> dist_ids = (*it_custom)->getDistributionIds();
		/* Container to save the distributions */
		vector<DistributionBase*> dist_ptrs;
		for(vector<DistributionId>::iterator it = dist_ids.begin() ; it != dist_ids.end() ; ++it) {
			map<DistributionId,DistributionBase*>::const_iterator it_dist_id = distribution_map.find((*it));
			if(it_dist_id == distribution_map.end())
				throw(DistributionBase::BadDistributionCreation((*it_custom)->getUserId(),
					  "Distribution id " + toString((*it)) + " does not exist"));
			else
				dist_ptrs.push_back((*it_dist_id).second);
		}
		/* Create the distribution */
		DistributionBase* dist_custom = new DistributionCustom(*it_custom,dist_ptrs);
		/* Push this distribution on the map */
		distribution_map[(*it_custom)->getUserId()] = dist_custom;
		/* Push it into the container */
		distributions.push_back(dist_custom);
	}

	/* Return created distributions */
	return distributions;
}

void DistributionFactory::registerDistribution(const DistributionBase& distribution) {
	constructor_table[distribution.getName()] = distribution.constructor();
}

DistributionBase::DistributionBase(const DistributionBaseObject* definition) : user_id(definition->getUserId()) {/* */};

std::ostream& operator<<(std::ostream& out, const DistributionBase& q) {
	out << "distribution = " << q.getUserId() << " ; type = " << q.getName() << " ;";
	q.print(out);
	return out;
}

/* Constructor from definition */
DistributionCustom::DistributionCustom(const DistributionBaseObject* definition,
		const vector<DistributionBase*>& distPtrs) : DistributionBase(definition) {
	const DistributionCustomObject* distObject = static_cast<const DistributionCustomObject*>(definition);
	/* Weights of each sampler */
	std::vector<double> weights = distObject->getWeights();
	/* Create sampler */
	distribution_sampler = new Sampler<DistributionBase*>(distPtrs,weights);
};

void DistributionCustom::print(std::ostream& out) const {
	out << endl;
	/* Get distributions */
	vector<DistributionBase*> distributions = distribution_sampler->getReactions();
	/* Reaction matrix */
	const double* reaction_matrix = distribution_sampler->getReactionMatrix();
	/* Print each distributions */
	size_t i = 0;
	for( ; i < distributions.size() - 1 ; ++i)
		out << Log::ident(3) << " - ( cdf = " << fixed << reaction_matrix[i] << " ) " << *distributions[i] << endl;
	/* Last one... */
	out << Log::ident(3) << " - ( cdf = " << 1.0 << " ) " << *distributions[i];
}

DistributionCustomObject::DistributionCustomObject(const std::string& type, const DistributionId& distid,
		   const std::vector<DistributionId>& samplersIds, const std::vector<double>& weights) :
	DistributionBaseObject(type,distid) , samplersIds(samplersIds) , weights(weights) {
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
