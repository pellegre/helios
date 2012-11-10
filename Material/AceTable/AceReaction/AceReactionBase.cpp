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

#include "AceReactionBase.hpp"
#include "ElasticScattering.hpp"
#include "FissionReaction.hpp"
#include "InelasticScattering.hpp"

namespace Helios {

using namespace AceReaction;

Reaction* AceReactionFactory::createReaction(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction) const {
	typedef Ace::AngularDistribution AceAngular;
	/* Get MT of the reaction to handle known cases */
	int mt = ace_reaction.getMt();

	if(mt == 2) {
		/*
		 * We know this is an elastic scattering reaction but we should peek over
		 * the angular distribution to create the correct template
		 */
		const Ace::AngularDistribution& ace_angular = ace_reaction.getAngular();

		if(ace_angular.getKind() == AceAngular::data)
			/* There is data on the angular distribution of this reaction */
			return new ElasticScattering<MuTable>(isotope, ace_reaction);

		else if(ace_angular.getKind() == AceAngular::isotropic)
			/* Isotropic sampling */
			return new ElasticScattering<MuIsotropic>(isotope, ace_reaction);

	} else if(mt == 18) {
		/* Fission reaction */
		return new Fission(isotope, ace_reaction);
	}

	throw(AceModule::AceError(isotope->getUserId(), "Reaction with mt = " + toString(mt) + " is not supported"));

	/* Generic inelastic reaction */
	return new InelasticScattering(isotope, ace_reaction);
}

}



