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
#include "AceReaction/AceReactionBase.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

double AceIsotope::energy_freegas_threshold = 400.0; /* By default, 400.0 kT*/
double AceIsotope::awr_freegas_threshold = 1.0;      /* By default, only H */

AceIsotope::AceIsotope(const Ace::ReactionContainer& _reactions, const ChildGrid* _child_grid) : Isotope(_reactions.name()),
	reactions(_reactions), aweight(_reactions.awr()), temperature(_reactions.temp()), child_grid(_child_grid),
	fission_reaction(0), secondary_sampler(0) {

	/* Total microscopic cross section of this isotope */
	total_xs = reactions.get_xs(1);

	/* Check if the isotope contains a fission cross section */
	fissile = reactions.check_all("18");
	/* Check for ith-chance fission */
	if(!fissile) fissile = reactions.check_all("19-21,38");

	/* Set the fission probability (in case the information is available) */
	if(fissile) {
		/* Get fission cross section */
		fission_xs = reactions.get_xs(18);
		/* Check size */
		assert(fission_xs.size() == total_xs.size());
		/* Set fission cross section */
		fission_reaction = getReaction(18);
	}

	/* Set the absorption cross section */
	absorption_xs = reactions.get_xs(27);
	/* Check size */
	if(absorption_xs.size() != 0)
		assert(absorption_xs.size() == total_xs.size());
	else
		absorption_xs = CrossSection(total_xs.size());

	/* Array for the secondary particle reaction sampler */
	vector<Reaction*> reaction_array;
	/* Arrays of XS of each reaction */
	vector<vector<double> > xs_array;

	/* Loop over the scattering reactions (skipping fission) */
	for(Ace::ReactionContainer::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it) {
		/* Get angular distribution type */
		int angular_data = (*it).getAngular().getKind();

		/* If the reaction does not contains angular data, we reach the end */
		if(angular_data == Ace::AngularDistribution::no_data) break;

		/* Get MT of the reaction */
		int mt = (*it).getMt();

		/* Check fission reaction */
		if(mt != 18) {
				/* Create reaction */
				reaction_array.push_back(getReaction(mt));
				/* Get cross section data */
				vector<double> xs_data(child_grid->size(),0.0);
				Ace::CrossSection xs = (*it).getXs();
				/* Fill container */
				for(size_t i = 0 ; i < xs_data.size() ; ++i)
					xs_data[i] = xs[i];
				/* Push it into the global array */
				xs_array.push_back(xs_data);
				cout << mt << " ";
		}
	}
	cout << endl;

	/* Create the sampler */
	secondary_sampler = new FactorSampler<Reaction*>(reaction_array, xs_array);
}

double AceIsotope::getAbsorptionProb(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	double abs = factor * (absorption_xs[idx + 1] - absorption_xs[idx]) + absorption_xs[idx];
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return abs / total;
}

double AceIsotope::getFissionProb(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	double fission = factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return fission / total;
}

double AceIsotope::getTotalXs(Energy& energy) const {
	double factor;
	size_t idx = child_grid->index(energy,factor);
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return total;
}

Reaction* AceIsotope::scatter(Particle& particle, Random& random) const {
	double factor;
	size_t idx = child_grid->index(particle.erg(), factor);
	return secondary_sampler->sample(idx, random.uniform(), factor);
};

Reaction* AceIsotope::getReaction(int mt) const {
	/* Static instance of the reaction factory */
	static AceReaction::AceReactionFactory reaction_factory;

	/* Get the reaction from the container */
	ReactionContainer::const_iterator it_reaction = reactions.get_mt(mt);

	if(it_reaction != reactions .end()) {
		/* Reaction exist */
		const NeutronReaction& ace_reaction = (*it_reaction);
		/* Return reaction */
		return reaction_factory.createReaction(this, ace_reaction);
	} else {
		/* Reaction can't be found */
		throw(AceModule::AceError(reactions.name(),"Reaction mt = " + toString(mt) + " does not exist"));
	}

}

void AceIsotope::print(std::ostream& out) const {
	out << "isotope = " <<  setw(9) << reactions.name()
		<< " ; awr = " << setw(9) << aweight << " ; temperature = " << temperature / Constant::boltz << " K ";
}

AceIsotope::~AceIsotope() {
	/* Delete fission reaction */
	delete fission_reaction;
	/* Delete sampler */
	delete secondary_sampler;
};

AceModule::AceModule(const std::vector<McObject*>& aceObjects, const McEnvironment* environment) : McModule(name(),environment) {
	/* Create master grid */
	master_grid = new MasterGrid();
	/* Loop over the definitions to create isotopes */
	for(vector<McObject*>::const_iterator it = aceObjects.begin() ; it != aceObjects.end() ; ++it) {
		/* Cast to AceObject */
		AceObject* ace_material = dynamic_cast<AceObject*>(*it);
		string isotope = ace_material->table_name;
		if (isotope_map.find(isotope) == isotope_map.end()) {
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

void AceModule::printIsotopes(std::ostream& out) const {
	out << "Ace Module  " << endl;
	out << " - Master grid size :" << master_grid->size() << endl;
	for(map<IsotopeId,AceIsotope*>::const_iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		out << " - " << *(*it).second << endl;
}

AceModule::~AceModule() {
	/* Delete isotopes */
	for(map<IsotopeId,AceIsotope*>::iterator it = isotope_map.begin() ; it != isotope_map.end() ; ++it)
		delete (*it).second;
	/* Delete master grid */
	delete master_grid;
}

} /* namespace Helios */
