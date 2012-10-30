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
 DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstdlib>

#include "NeutronReaction.hpp"
#include "Utils.hpp"

using namespace ACE;
using namespace std;

static const string& tab = "   ";

void NeutronReaction::printInformation(std::ostream& out) const {
	out << tab << tab << "[@] MT = " << getMT() << endl;
	out << tab << tab << "[@] Number of energies  : " << getXS().xs_data.size() << endl;
	out << tab << tab << "[@] Reaction parameters : ";
	out << "Q = " << fixed << getQ() << " MeV "<< " ; ";
	int ty = tyr_dist.getTYR();
	string coord = "";
	if(ty < 0) coord = "(CM system)";
	if(ty > 0) coord = "(LAB system)";
	ty = abs(ty);
	if(ty == 19) {
		out << "Neutron released = Fission (information on the NU block)";
	} else if(ty >= 100) {
		out << "Neutron released = Energy–dependent, information on the DLW block " << coord;
	} else {
		out << "Neutron released = " << ty << " " << coord;
	}
	out << endl;

	/* Information about the angular distribution */
	string andist = "[@] Angular distribution = ";
	if(adist.getKind() == AngularDistribution::isotropic)
		andist += "Isotropic";
	if(adist.getKind() == AngularDistribution::law44)
		andist += "From DLW block, Law 44";
	if(adist.getKind() == AngularDistribution::data)
		andist += "Data on AND block";
	if(adist.getKind() == AngularDistribution::no_data)
		andist += "None";

	out << tab << tab << andist;
	out << endl;

	/* Information about the energy distribution */
	string endist = "[@] Energy distribution = ";
	if(edist.getKind() == EnergyDistribution::no_data)
		endist += "None";
	else {
		vector<int> nlaws = edist.getLaws();
		for(size_t i = 0 ; i < nlaws.size() ; i++)
			endist += "Law " + toString(nlaws[i]) + " - " + EnergyDistribution::energy_laws[nlaws[i]] + " ; ";
	}

	out << tab << tab << endist;
	out << endl;
}
