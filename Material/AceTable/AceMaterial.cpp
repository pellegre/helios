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

#include "AceMaterial.hpp"
#include "AceReader/NeutronTable.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

//virtual vector<Material*> AceMaterialFactory::createMaterials(const vector<MaterialObject*>& definitions) const {
//	/* Map to hold the isotopes created */
//	map<string,AceIsotope*> isotopes;
//
//	/* Loop over the definitions to create isotopes */
//	for(vector<MaterialObject*>::const_iterator it = definitions.begin() ; it != definitions.end() ; ++it) {
//		/* Cast to AceObject */
//		AceMaterialObject* ace_material = dynamic_cast<AceMaterialObject*>(*it);
//		map<std::string,double>::const_iterator iso = ace_material->isotopes.begin();
//		for(; iso != ace_material->isotopes.end() ; ++iso) {
//			string isotope = (*iso).first;
//			if (isotopes.find(isotope) == isotopes.end()) {
//				/* Get the neutron table using the AceReader */
//				NeutronTable* table = dynamic_cast<NeutronTable*>(AceReader::getTable(isotope));
//				/* Create child grid */
//				const ChildGrid* child_grid = master_grid->pushGrid(table->getEnergyGrid().begin(), table->getEnergyGrid().end());
//				/* Create isotope and update the map */
//				isotopes[isotope] = new AceIsotope(table->getReactions(), child_grid);
//				/* Delete table, we don't need it anymore */
//				delete table;
//			}
//		}
//	}
//}

} /* namespace Helios */
