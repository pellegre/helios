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

#include "FissionPolicy.hpp"
#include "../AceReaction/FissionReaction.hpp"
#include "../AceIsotopeBase.hpp"
#include "../AceModule.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

/* Create NU sampler based on the information on the ACE data */
static AceReaction::NuSampler* buildNuSampler(const Ace::NUBlock::NuData* nu_data) {
	typedef Ace::NUBlock AceNu;
	/* Get type */
	int type = nu_data->getType();
	/* Polynomial type */
	if(type == AceNu::flag_pol)
		return new AceReaction::PolynomialNu(dynamic_cast<const AceNu::Polynomial*>(nu_data));
	/* Tabular type */
	return new AceReaction::TabularNu(dynamic_cast<const AceNu::Tabular*>(nu_data));
}

FissilePolicyBase::FissilePolicyBase(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
		fission_xs(CrossSection(_table.getEnergyGrid().size())), child_grid(_child_grid) {
	/* Null cross section */
}

/* Get fission cross section */
double FissilePolicyBase::getFissionXs(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy, factor);
	double fission = factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
	return fission;
}

TotalNuFission::TotalNuFission(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
	FissilePolicyBase(_isotope, _table, _child_grid) {
	/* Get reaction */
	const ReactionContainer& reactions(_table.getReactions());
	/* Get fission cross section */
	fission_xs = reactions.get_xs(18);
	/* Set fission cross section */
	fission_reaction = _isotope->getReaction(18);

	/* Get fission reaction */
	const Ace::NeutronReaction& ace_reaction = (*reactions.get_mt(18));
	/* Get distribution of emerging particles from the NU-block */
	const Ace::NUBlock* nu_block = _table.block<NUBlock>();

	/* Get the NU data related to this fission reaction */
	vector<Ace::NUBlock::NuData*> nu_data = nu_block->clone();

	/* If total NU is available in NU-block, get it from here */
	if(nu_data.size() >= 2)
		total_nu = buildNuSampler(nu_data[1]);
	else if(nu_data.size() == 1)
		total_nu = buildNuSampler(nu_data[0]);
	else
		throw(AceModule::AceError(_isotope->getUserId(), "Cannot create reaction for mt = " + toString(ace_reaction.getMt()) +
			" : Information in NU block is not available" ));
};

TotalNuChanceFission::TotalNuChanceFission(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
		FissilePolicyBase(_isotope, _table, _child_grid) {
	/* Get reaction */
	const ReactionContainer& reactions(_table.getReactions());
	/* Get fission cross section */
	fission_xs = reactions.get_xs(18);

	int fission_mt(19);

	/* MT of chance fission reaction */
	int mts[4] = {19, 20, 21, 38};
	vector<int> chance_mts(mts, mts + 4);

	/* Array for the secondary particle reaction sampler */
	vector<pair<Reaction*,const CrossSection*> > chance_array;

	/* Push chance reactions */
	for(vector<int>::const_iterator it = chance_mts.begin() ; it != chance_mts.end() ; ++it) {
		/* Get MT */
		int mt = (*it);

		/* Get the reaction from the container */
		ReactionContainer::const_iterator it_reaction = reactions.get_mt(mt);
		if(it_reaction != reactions.end()) {
			/* Reaction exist */
			const NeutronReaction& ace_reaction = (*it_reaction);
			/* Push reaction */
			chance_array.push_back( make_pair(_isotope->getReaction(mt), &(ace_reaction.getXs()) ));
		}
	}

	/* Set fission cross section */
	fission_reaction = new AceReaction::ChanceFission(chance_array, fission_xs, _child_grid);

	/* Put fission MT */
	fission_mt = (*chance_array.begin()).first->getId();

	/* Get distribution of emerging particles from the NU-block */
	const Ace::NUBlock* nu_block = _table.block<NUBlock>();

	/* Get the NU data related to this fission reaction */
	vector<Ace::NUBlock::NuData*> nu_data = nu_block->clone();

	/* If total NU is available in NU-block, get it from here */
	if(nu_data.size() >= 2)
		total_nu = buildNuSampler(nu_data[1]);
	else if(nu_data.size() == 1)
		total_nu = buildNuSampler(nu_data[0]);
	else
		throw(AceModule::AceError(_isotope->getUserId(), "Cannot create reaction for fission "
				": Information in NU block is not available" ));
}

}


