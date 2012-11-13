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

#include "InelasticScattering.hpp"

namespace Helios {

using namespace AceReaction;

MuSampler* GenericReaction::buildMuSampler(const Ace::AngularDistribution& ace_angular) {
	typedef Ace::AngularDistribution AceAngular;

	if(ace_angular.getKind() == AceAngular::data)
		/* There is data on the angular distribution of this reaction */
		return new MuTable(ace_angular);

	else if(ace_angular.getKind() == AceAngular::isotropic)
		/* Isotropic sampling */
		return new MuIsotropic(ace_angular);

	else if(ace_angular.getKind() == AceAngular::law44)
		/* MU sampling is done on the energy distribution */
		return 0;

	throw(GeneralError("No angular distribution defined"));
}

EnergySamplerBase* GenericReaction::buildEnergySampler(const Ace::EnergyDistribution& ace_energy) {
	typedef Ace::EnergyDistribution AceEnergy;
	/* Sampler factory */
	static EnergySamplerFactory sampler_factory;
	/* Check if there is data inside the table */
	if(ace_energy.getKind() == AceEnergy::data) {
		/* We should create an energy sampler using ACE energy laws */
		return sampler_factory.createSampler(ace_energy);
	}
	/* No data, probably an elastic scattering reaction */
	return 0;
}

GenericReaction::GenericReaction(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction) :
		Reaction(ace_reaction.getMt()), mu_sampler(0), energy_sampler(0) {
	/* Build MU sampler */
	try {
		mu_sampler = buildMuSampler(ace_reaction.getAngular());
	} catch (exception& error) {
		throw(AceModule::AceError(isotope->getUserId(),
				"Cannot create reaction for mt = " + toString(ace_reaction.getMt()) + " : " + error.what()));
	}
	/* Build energy sampler */
	try {
		energy_sampler = buildEnergySampler(ace_reaction.getEnergy());
	} catch (exception& error) {
		throw(AceModule::AceError(isotope->getUserId(),
				"Cannot create reaction for mt = " + toString(ace_reaction.getMt()) + " : " + error.what()));
	}
}

void GenericReaction::print(std::ostream& out) const {
	Log::printLine(out,"*");
	out << endl;
	/* Print MU sampler */
	if(mu_sampler) {
		out << "[+] Angular Distribution : " << endl;
		mu_sampler->print(out);
	}
	Log::printLine(out,"*");
	out << endl;
	/* Print energy sampler */
	if(energy_sampler) {
		out << "[+] Energy Distribution : " << endl;
		energy_sampler->print(out);
	}
	Log::printLine(out,"*");
	out << endl;
}

GenericReaction::~GenericReaction() {
	/* Delete MU sampler */
	delete mu_sampler;
	/* Delete energy sampler */
	delete energy_sampler;
}
} /* namespace Helios */
