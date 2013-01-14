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

#include "AceModule.hpp"
#include "AceReader/ACEReader.hpp"
#include "AceReader/NeutronTable.hpp"
#include "AceReader/Conf.hpp"
#include "AceReaction/AceReactionBase.hpp"
#include "AceReaction/FissionReaction.hpp"
#include "../../Common/XsSampler.hpp"
#include "../../Environment/McEnvironment.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

AceModule::AceModule(const std::vector<McObject*>& aceObjects, const McEnvironment* environment) : McModule(name(),environment) {
	Log::bok() << "Initializing Ace Module " << Log::endl;
	/* Try to get the location of the XS data from the environment */
	if(environment->isSet("xs_data"))
		Ace::Conf::DATAPATH = environment->getSetting<string>("xs_data","value");

	/* Print information about the Ace reader */
	Log::msg() << left << Log::ident(1) << " - Using xsdir from directory " << Ace::Conf::DATAPATH << Log::endl;

	/* Create master grid */
	master_grid = new MasterGrid();
	/* Loop over the definitions to create isotopes */
	for(vector<McObject*>::const_iterator it = aceObjects.begin() ; it != aceObjects.end() ; ++it) {
		/* Cast to AceObject */
		AceObject* ace_material = dynamic_cast<AceObject*>(*it);
		string isotope = ace_material->table_name;
		if (isotope_map.find(isotope) == isotope_map.end()) {
			/* Print information about the isotope */
			Log::msg() << left << Log::ident(2) << "  Reading isotope ";
			Log::color<Log::COLOR_BOLDWHITE>() << isotope << Log::endl;
			/* Get the neutron table using the AceReader */
			NeutronTable* table = dynamic_cast<NeutronTable*>(AceReader::getTable(isotope));
			/* Create child grid */
			const ChildGrid* child_grid = master_grid->pushGrid(table->getEnergyGrid().begin(), table->getEnergyGrid().end());
			/* Create isotope */
			AceIsotope* new_isotope = new AceIsotope(table->getReactions(), child_grid);
			/* Update the map */
			isotope_map[isotope] = new_isotope;
			/* Push isotope into the container */
			isotopes.push_back(new_isotope);
			/* Delete table, we don't need it anymore */
			delete table;
		}
	}
	Log::msg() << left << Log::ident(1) << " - Setting up master grid " << Log::endl;
	/* Setup master grid */
	master_grid->setup();

	/* Update maps */
	for(size_t i = 0; i < isotopes.size() ; ++i) {
		/* Set internal / unique index */
		isotopes[i]->setInternalId(i);
		IsotopeId id = isotopes[i]->getUserId();
		/* Update material map */
		internal_isotope_map[isotopes[i]->getUserId()] = isotopes[i]->getInternalId();
	}

}

template<>
std::vector<AceIsotope*> AceModule::getObject<AceIsotope>(const UserId& id) const {
	map<std::string,AceIsotope*>::const_iterator iso = isotope_map.find(id);
	if(iso == isotope_map.end()) {
		throw AceError(id,"Isotope does not exist inside the ace module");
	} else {
		vector<AceIsotope*> v;
		v.push_back((*iso).second);
		return v;
	}
}

void AceModule::print(std::ostream& out) const {
	out << " - Master grid size :" << master_grid->size() << endl;
	for(map<IsotopeId,AceIsotope*>::const_iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		out << " - " << *(*it).second << endl;
	out << endl;
}

AceModule::~AceModule() {
	/* Delete isotopes */
	for(map<IsotopeId,AceIsotope*>::iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		delete (*it).second;
	/* Delete master grid */
	delete master_grid;
}

} /* namespace Helios */
