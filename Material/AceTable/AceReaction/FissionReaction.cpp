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

#include "FissionReaction.hpp"

namespace Helios {

using namespace AceReaction;

/* Create NU sampler based on the information on the ACE data */
static NuSampler* buildNuSampler(const Ace::NUBlock::NuData* nu_data) {
	typedef Ace::NUBlock AceNu;
	/* Get type */
	int type = nu_data->getType();
	/* Polynomial type */
	if(type == AceNu::flag_pol)
		return new PolynomialNu(dynamic_cast<const AceNu::Polynomial*>(nu_data));
	/* Tabular type */
	return new TabularNu(dynamic_cast<const AceNu::Tabular*>(nu_data));
}

Fission::Fission(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction) :
		GenericReaction(isotope, ace_reaction), prompt_nu(0) {

	/* Get distribution of emerging particles */
	const Ace::TyrDistribution& tyr = ace_reaction.getTyr();
	/* Sanity check */
	assert(tyr.getType() == Ace::TyrDistribution::fission);
	/* Get the NU data related to this fission reaction */
	vector<Ace::NUBlock::NuData*> nu_data = tyr.getFission();

	/* TODO - For now just sample particles with prompt spectrum */
	if(nu_data.size() >= 2)
		prompt_nu = buildNuSampler(nu_data[1]);
	else if(nu_data.size() == 1)
		prompt_nu = buildNuSampler(nu_data[0]);
	else
		throw(AceModule::AceError(isotope->getUserId(),
			"Cannot create reaction for mt = " + toString(ace_reaction.getMt()) + " : Information in NU block is not available" ));
}

void Fission::print(std::ostream& out) const {
	out << " - Fission Reaction" << endl;
	Log::printLine(out,"*");
	out << endl;
	prompt_nu->print(out);
	/* Print the cosine and energy sampler */
	GenericReaction::print(out);
}

Fission::~Fission() {
	delete prompt_nu;
}

} /* namespace Helios */
