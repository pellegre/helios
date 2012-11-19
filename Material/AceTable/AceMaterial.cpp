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
#include "../../Environment/McEnvironment.hpp"

using namespace std;
using namespace Ace;

namespace Helios {

static void normalize(map<string,double>& isotopes_fraction) {
	map<string,double>::iterator it = isotopes_fraction.begin();
	double total = 0.0;
	/* Get total */
	for(; it != isotopes_fraction.end() ; ++it)
		total += (*it).second;
	/* Normalize values */
	for(it = isotopes_fraction.begin(); it != isotopes_fraction.end() ; ++it)
		(*it).second /= total;
}

/* Calculate the average atomic number, and set the isotope map on the material */
double AceMaterial::setIsotopeMap(string& type, map<string,double> isotopes_fraction, const map<string,AceIsotope*>& isotopes) {
	/* Normalize fractions */
	normalize(isotopes_fraction);

	/* Accumulator to calculate the average atomic number of the material */
	double accum = 0.0;
	std::map<std::string,double>::iterator it = isotopes_fraction.begin();
	for(; it != isotopes_fraction.end() ; ++it) {
		/* Get isotope name */
		string name = (*it).first;
		/* This shouldn't happen at this stage, but let's check anyway */
		if(isotopes.find(name) == isotopes.end())
			throw(Material::BadMaterialCreation(getUserId(),"Isotope " + name + " does not exist"));

		/* Get isotope */
		AceIsotope* ace_isotope = (*isotopes.find(name)).second;
		/* Get fraction */
		double fraction = isotopes_fraction[name];

		/* Check the type of the fraction */
		if(type == "atom") {
			/* Accumulate */
			accum += fraction * ace_isotope->getAwr();
			/* Save isotope */
			isotope_map.insert(pair<string,IsotopeData>(name,IsotopeData(0.0, fraction, ace_isotope)));
		}
		else if (type == "weight") {
			/* Accumulate */
			accum += fraction / ace_isotope->getAwr();
			/* Save isotope */
			isotope_map.insert(pair<string,IsotopeData>(name,IsotopeData(fraction, 0.0, ace_isotope)));
		}
		else
			throw(Material::BadMaterialCreation(getUserId(),"Fraction type " + type + " not recognized"));
	}

	/* Calculate average atomic number */
	double average_atomic = 0.0;
	if(type == "atom")
		average_atomic = accum;
	else if(type == "weight")
		average_atomic = 1.0 / accum;

	/* Now update each isotope fraction */
	std::map<std::string,IsotopeData>::iterator iso = isotope_map.begin();
	for(; iso != isotope_map.end() ; ++iso) {
		double awr = (*iso).second.isotope->getAwr();
		if(type == "atom")
			(*iso).second.mass_fraction = (*iso).second.atomic_fraction * awr / average_atomic;
		else if(type == "weight")
			(*iso).second.atomic_fraction = (*iso).second.mass_fraction * average_atomic / awr;
	}

	/* Return average atomic number */
	return average_atomic;
}

AceMaterial::AceMaterial(const AceMaterialObject* definition) :
		 Material(definition)
		,master_grid(definition->getEnvironment()->getModule<AceModule>()->getMasterGrid())
		,total_xs(master_grid->size(),0.0) {

	/* Type of isotope fractions */
	string type = definition->fraction;
	/* Isotope fractions */
	map<string,double> isotope_fraction = definition->isotopes;

	if(isotope_fraction.size() == 0)
		throw(Material::BadMaterialCreation(getUserId(),"Material does not contain any isotope"));

	/* Get isotope map from the ACE module */
	map<string,AceIsotope*> isotopes = definition->getEnvironment()->getModule<AceModule>()->getIsotopeMap();

	/* Get average atomic number and set the isotope map */
	double average_atomic = setIsotopeMap(type, isotope_fraction, isotopes);

	/* Set densities */
	string units = definition->units;
	if(units == "g/cm3") {
		/* Mass density */
		rho = definition->density;
		atom = rho * Constant::avogadro / average_atomic;
	} else if(units == "atom/b-cm") {
		/* Atomic density */
		atom = definition->density;
		rho = atom * average_atomic / Constant::avogadro;
	} else
		throw(Material::BadMaterialCreation(getUserId(),"Unit " + units + " not recognized in density"));

	/* -- Setup the isotope sampler and the mean free path of the material */

	/* Array for the isotope sampler */
	vector<AceIsotope*> isotope_array(isotope_map.size());
	/* Arrays of XS of each isotope*/
	vector<vector<double> > xs_array(isotope_map.size(), vector<double>(master_grid->size(),0.0));

	/* Process data of each isotope */
	std::map<std::string,IsotopeData>::iterator iso = isotope_map.begin();
	/* Isotope counter */
	size_t counter = 0;
	for(; iso != isotope_map.end() ; ++iso) {
		/* Get isotope */
		AceIsotope* ace_isotope = (*iso).second.isotope;
		/* Get atomic density */
		double density = (*iso).second.atomic_fraction * atom;
		/* Push isotope to the array*/
		isotope_array[counter] = ace_isotope;
		/* Set the XS array for this isotope */
		Energy energy(0,0.0);
		for(size_t i = 0 ; i < master_grid->size() ; ++i) {
			/* Set the energy and leave the index alone (faster interpolation) */
			energy.second = (*master_grid)[i];
			/* Set isotope cross section on this material */
			double total = density * ace_isotope->getTotalXs(energy);
			xs_array[counter][i] = total;
			/* Contribution to the mean free path */
			total_xs[i] += total;
		}
		/* Increment isotope */
		++counter;
	}

	/* Set the isotope sampler */
	isotope_sampler = new FactorSampler<AceIsotope*>(isotope_array, xs_array, false);

}

double AceMaterial::getMeanFreePath(Energy& energy) const {
	double factor = master_grid->interpolate(energy);
	size_t idx = energy.first;
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return 1.0 / total;
}

const Isotope* AceMaterial::getIsotope(Energy& energy, Random& random) const {
	double factor = master_grid->interpolate(energy);
	size_t idx = energy.first;
	double total = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
	return isotope_sampler->sample(idx,total * random.uniform(), factor);
}

AceMaterial::~AceMaterial() {
	delete isotope_sampler;
};

void AceMaterial::print(std::ostream& out) const {
	out << scientific;
	/* Print material information */
	out << Log::ident(1) << " - density = " << setw(9) << rho << " g/cm3 " << endl;
	out << Log::ident(1) << " - density = " << setw(9) << atom << " atom/b-cm " << endl;
	/* Print isotope information */
	std::map<std::string,IsotopeData>::const_iterator iso = isotope_map.begin();
	for(; iso != isotope_map.end() ; ++iso)
		out << Log::ident(2) << "(mass fraction = " << setw(9) << (*iso).second.mass_fraction << " ; "
		<< "atomic fraction = " << setw(9) << (*iso).second.atomic_fraction << ") "
		<< *(*iso).second.isotope << endl;

}

vector<Material*> AceMaterialFactory::createMaterials(const vector<MaterialObject*>& definitions) const {
	/* Container of new materials */
	vector<Material*> materials;

	/* Push materials */
	for(vector<MaterialObject*>::const_iterator it = definitions.begin() ;  it != definitions.end() ; ++it) {
		const AceMaterialObject* new_ace = static_cast<const AceMaterialObject*>((*it));
		AceMaterial* newMaterial = new AceMaterial(new_ace);
		/* Print additional information */
		Log::msg() << left << Log::ident(2) << "  Creating material ";
		Log::color<Log::COLOR_BOLDWHITE>() << newMaterial->getUserId() << Log::endl;
		/* Push material */
		materials.push_back(newMaterial);
	}

	/* Return container */
	return materials;
}

} /* namespace Helios */
