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

#include "EnergySampler.hpp"
#include "EnergyLaws/EnergyLaws.hpp"

namespace Helios {

using namespace AceReaction;

/* Create law */
static EnergySamplerBase* createLaw(const Ace::EnergyDistribution::EnergyLaw* ace_law) {
	/* Get law */
	int law = ace_law->getLaw();

	if(law == 1) {
		/* Level Scattering */
		return new EnergySampler<EnergyLaw1>(ace_law);
	}
	else if(law == 3) {
		/* Level Scattering */
		return new EnergySampler<EnergyLaw3>(ace_law);
	}
	else if(law == 4) {
		/* Continuous Tabular Distribution */
		return new EnergySampler<EnergyLaw4>(ace_law);
	}
	else if(law == 7) {
		/* Continuous Tabular Distribution */
		return new EnergySampler<EnergyLaw7>(ace_law);
	}
	else if(law == 9) {
		/* Continuous Tabular Distribution */
		return new EnergySampler<EnergyLaw9>(ace_law);
	}
	else if(law == 44) {
		/* Continuous Tabular Distribution */
		return new EnergySampler<EnergyLaw44>(ace_law);
	}
	else if(law == 61) {
		/* Continuous Tabular Distribution */
		return new EnergySampler<EnergyLaw61>(ace_law);
	}
	/* No law */
	throw(EnergySamplerBase::BadEnergySamplerCreation("Energy law " + toString(law) + " is not supported"));
}

EnergySamplerBase* EnergySamplerFactory::createSampler(const Ace::EnergyDistribution& ace_data) {
	typedef Ace::EnergyDistribution::EnergyLaw EnergyLaw;
	if(ace_data.laws.size() == 1)
		return createLaw(ace_data.laws[0]);
	return new MultipleLawsSampler(ace_data.laws);
}

MultipleLawsSampler::MultipleLawsSampler(const vector<EnergyLaw*>& laws) {
	for(vector<EnergyLaw*>::const_iterator it = laws.begin() ; it != laws.end() ; ++it)
		/* Create law data and push it into the container */
		law_table.push_back(LawValidity((*it)->energy, (*it)->prob, createLaw((*it))));
}

void MultipleLawsSampler::print(std::ostream& out) const {
	for(std::vector<LawValidity>::const_iterator it = law_table.begin() ; it != law_table.end() ; ++it) {
		out << " energies = ";
		for(size_t i = 0 ; i < (*it).energy.size() ; ++i)
			out << scientific << (*it).energy[i] << " ";
		out << endl; out << " probabilities = ";
		for(size_t i = 0 ; i < (*it).prob.size() ; ++i)
			out << scientific << (*it).prob[i] << " ";
		out << endl;
		(*it).energy_law->print(out);
	}
}


}



