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


#include "AceDistribution.hpp"
#include "../../Environment/McEnvironment.hpp"

using namespace std;

namespace Helios {

AceDistribution::AceDistribution(const DistributionBaseObject* definition) : DistributionBase(definition), ace_reaction(0) {
	/* Cast definition to this type */
	const DistributionAceObject* ace_definition = static_cast<const DistributionAceObject*>(definition);
	/* Get environment */
	const McEnvironment* environment = ace_definition->getEnvironment();
	/* Get isotope */
	isotope = environment->getObject<AceModule,AceIsotope>(ace_definition->isotope_id)[0];
	/* Get the reaction */
	ace_reaction = isotope->getReaction(ace_definition->reaction_id);
}

void AceDistribution::print(std::ostream& out) const {
	/* Just print the isotope */
	out << *isotope << endl;
}

}


