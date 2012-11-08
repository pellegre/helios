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

#include "MuSampler.hpp"

namespace Helios {

using namespace AceReaction;

/* Cosine table builder */
CosineTable* MuTable::tableBuilder(const AceAngular* ace_array) {
	typedef Ace::AngularDistribution Ang;
	typedef Ang::TableType TableType;
	TableType type = ace_array->getType();
	if(type == Ang::isotropic_table)
		return new Isotropic();
	else if(type == Ang::equibins_table)
		return new EquiBins(static_cast<const AceEquiBins*>(ace_array));
	else if(type == Ang::tabular_table)
		return new Tabular(static_cast<const AceTabular*>(ace_array));
	return 0;
}

/* Constructor */
MuTable::MuTable(const Ace::AngularDistribution& ace_data) {
	energies = ace_data.energy;
	/* Sanity check */
	assert(ace_data.adist.size() == energies.size());
	/* Create the tables */
	for(vector<AceAngular*>::const_iterator it = ace_data.adist.begin() ; it != ace_data.adist.end() ; ++it)
		cosine_table.push_back(tableBuilder(*it));
}

}


