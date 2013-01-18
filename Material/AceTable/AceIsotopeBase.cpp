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

#include "AceIsotopeBase.hpp"
#include "AceReader/ACEReader.hpp"
#include "AceReader/NeutronTable.hpp"
#include "AceReader/Conf.hpp"
#include "AceReader/Ace.hpp"
#include "AceReaction/AceReactionBase.hpp"
#include "AceReaction/FissionReaction.hpp"
#include "../../Common/XsSampler.hpp"
#include "../../Environment/McEnvironment.hpp"

#include "AceIsotopes/AceIsotope.hpp"
#include "AceIsotopes/FissionPolicy.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

AceIsotopeBase* AceIsotopeFactory::createIsotope(const Ace::NeutronTable& table) const {
	/* Create child grid */
	const ChildGrid* child_grid = master_grid->pushGrid(table.getEnergyGrid().begin(), table.getEnergyGrid().end());

	/* Get distribution of emerging particles from the NU-block */
	const Ace::NUBlock* nu_block = table.block<NUBlock>();

	/* Check if NU block is available in the table */
	if(nu_block) {
		/* Create isotope*/
		AceIsotopeBase* isotope(0);

		/* Get reaction */
		const ReactionContainer& reactions(table.getReactions());

		/* Get distribution of delayed emerging particles from the NU-block */
		const Ace::DLYBlock* del_block = table.block<DLYBlock>();

		/* Check if delayed neutron information is available */
		if(del_block) {

			/* Check the available cross sections on the table */
			if(reactions.check_all("18")) {
				isotope = new AceIsotope<DelayedFissionSampler<SingleFissionReaction, DelayedNu> >(table, child_grid);
			} else if(reactions.check_all("19-21,38")) {
				isotope = new AceIsotope<DelayedFissionSampler<ChanceFissionReaction, DelayedNu> >(table, child_grid);
			} else {
				throw(AceModule::AceError(table.getName(), "Cannot create fission reaction : Fission cross section is not available" ));
			}

		} else {

			// Print warning
			Log::warn() << left << "Delayed neutron information is not available for isotope " << table.getName() << endl;

			/* Check the available cross sections on the table */
			if(reactions.check_all("18")) {
				isotope = new AceIsotope<PromptFissionSampler<SingleFissionReaction, TotalNu> >(table, child_grid);
			} else if(reactions.check_all("19-21,38")) {
				isotope = new AceIsotope<PromptFissionSampler<ChanceFissionReaction, TotalNu> >(table, child_grid);
			} else {
				throw(AceModule::AceError(table.getName(), "Cannot create fission reaction : Fission cross section is not available" ));
			}

		}

		/* Set fissile flag in the isotope */
		isotope->fissile = true;
		/* Fissile isotope */
		return isotope;
	}

	/* Create isotope */
	return new AceIsotope<NonFissile>(table, child_grid);
}

double AceIsotopeBase::energy_freegas_threshold = 400.0; /* By default, 400.0 kT*/
double AceIsotopeBase::awr_freegas_threshold = 1.0;      /* By default, only H */

AceIsotopeBase::AceIsotopeBase(const Ace::NeutronTable& _table, const ChildGrid* _child_grid) : Isotope(_table.getReactions().name()),
	reactions(_table.getReactions()), aweight(reactions.awr()), temperature(reactions.temp()), child_grid(_child_grid),
	secondary_sampler(0) {

	/* Total microscopic cross section of this isotope */
	total_xs = reactions.get_xs(1);

	/* Elastic cross section */
	elastic_xs = reactions.get_xs(2);
	/* Set elastic reaction */
	elastic_scattering = getReaction(2);

	/* Set the absorption cross section */
	absorption_xs = reactions.get_xs(27);
	/* Check size */
	if(absorption_xs.size() != 0) {
		if(absorption_xs.size() != total_xs.size())
			throw(AceModule::AceError(getUserId(), "Absorption and total cross section don't have the same size"));
	} else {
		/* The only case when this happens is with 1002 isotope */
		absorption_xs = CrossSection(total_xs.size());
	}

	/* 	Calculate inelastic cross section */
	inelastic_xs = total_xs - absorption_xs - elastic_xs;

	/* Array for the secondary particle reaction sampler */
	vector<pair<Reaction*,const CrossSection*> > reaction_array;

	/* Loop over the scattering reactions (skipping fission) */
	for(Ace::ReactionContainer::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it) {
		/* Get angular distribution type */
		int angular_data = (*it).getAngular().getKind();

		/* If the reaction does not contains angular data, we reach the end of "secondary" particle reactions */
		if(angular_data == Ace::AngularDistribution::no_data) break;

		/* Get MT of the reaction */
		int mt = (*it).getMt();

		/* We shouldn't include elastic and fission here */
		if((mt < 18 || mt > 21) && mt != 38 && mt != 2) {
				/* Create reaction */
				reaction_array.push_back(make_pair(getReaction(mt), &(*it).getXs()));
		}
	}

	/* Create the sampler */
	if(reaction_array.size() > 0)
		secondary_sampler = new XsSampler<Reaction*>(reaction_array);

}

/* Auxiliary function to get the probability of a reaction */
double AceIsotopeBase::getProb(Energy& energy, const Ace::CrossSection& xs) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	double prob = factor * (xs[idx + 1] - xs[idx]) + xs[idx];
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return prob / total;
}

double AceIsotopeBase::getAbsorptionProb(Energy& energy) const {
	return getProb(energy, absorption_xs);
}

double AceIsotopeBase::getFissionProb(Energy& energy) const {
	return getFissionXs(energy) / getTotalXs(energy);
}

double AceIsotopeBase::getElasticProb(Energy& energy) const {
	return getProb(energy, elastic_xs);
}

double AceIsotopeBase::getTotalXs(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return total;
}

Reaction* AceIsotopeBase::inelastic(Energy& energy, Random& random) const {
	/*
	 * Check if there is a sampler. This method shouldn't be executed when
	 * there aren't non-elastic reactions, but who knows...
	 */
	if(!secondary_sampler) return elastic_scattering;
	/* Get inelastic reaction from the sampler */
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double inel = factor * (inelastic_xs[idx + 1] - inelastic_xs[idx]) + inelastic_xs[idx];
	return secondary_sampler->sample(idx, inel * random.uniform(), factor);
};

Reaction* AceIsotopeBase::getReaction(InternalId mt) {
	/* Static instance of the reaction factory */
	static AceReaction::AceReactionFactory reaction_factory;

	/* Check on local map */
	map<int,Reaction*>::const_iterator rea = reaction_map.find(mt);
	if(rea != reaction_map.end())
		return (*rea).second;

	/* Get the reaction from the container */
	ReactionContainer::const_iterator it_reaction = reactions.get_mt(mt);

	if(it_reaction != reactions.end()) {
		/* Reaction exist */
		const NeutronReaction& ace_reaction = (*it_reaction);
		/* Return reaction */
		Reaction* reaction = reaction_factory.createReaction(this, ace_reaction);
		/* Push reaction into local map */
		reaction_map[mt] = reaction;
		/* And return it... */
		return reaction;
	} else {
		/* Reaction can't be found */
		throw(AceModule::AceError(reactions.name(),"Reaction mt = " + toString(mt) + " does not exist"));
	}

}

void AceIsotopeBase::print(std::ostream& out) const {
	out << "isotope = " <<  setw(9) << reactions.name()
		<< " ; awr = " << setw(9) << aweight << " ; temperature = " << temperature / Constant::boltz << " K ";
}

AceIsotopeBase::~AceIsotopeBase() {
	/* Delete reactions */
	for(map<int,Reaction*>::const_iterator it = reaction_map.begin() ; it != reaction_map.end() ; ++it)
		delete (*it).second;
	/* Delete sampler */
	delete secondary_sampler;
};

}
