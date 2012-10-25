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

#include "MacroXs.hpp"

using namespace std;

namespace Helios {

MacroXs::MacroXs(const MacroXsObject* definition, int number_groups) :
		         Material(definition), ngroups(number_groups), mfp(number_groups) {
	/* Get constants */
	map<string,vector<double> > constant = definition->getConstant();
	/* Get the number of groups and do some error checking */
	map<string,vector<double> >::const_iterator it_xs = constant.begin();
	int ngroup = number_groups;
	for(; it_xs != constant.end() ; ++it_xs) {
		string xs_name = (*it_xs).first;
		if(xs_name == "sigma_s") {
			int size = (*it_xs).second.size();
			if(size != ngroup * ngroup) {
				throw Material::BadMaterialCreation(getUserId(),
						"Inconsistent number of groups in constant *" + xs_name + "*");
			}
		} else {
			int size = (*it_xs).second.size();
			if(size != ngroup) {
				throw Material::BadMaterialCreation(getUserId(),
						"Inconsistent number of groups in constant *" + xs_name + "*");
			}
		}
	}

	/* Now set the reactions */
	map<Reaction*,vector<double> > reaction_map;

	/* ---- Capture reaction */
	vector<double> sigma_f = constant["sigma_f"];
	vector<double> sigma_a = constant["sigma_a"];
	vector<double> sigma_c(ngroups);
	for(size_t i = 0 ; i < ngroups ; ++i)
		sigma_c[i] = sigma_a[i] - sigma_f[i];
	reaction_map[new Absorption()] = sigma_c;

	/* ---- Scattering cross section */
	vector<double> sigma_s(ngroups);
	vector<double> scattering_matrix = constant["sigma_s"];
	for(size_t i = 0 ; i < ngroups ; ++i) {
		double total_scat = 0;
		for(size_t j = 0 ; j < ngroups ; ++j)
			total_scat += scattering_matrix[i*ngroups + j];
		sigma_s[i] = total_scat;
	}
	reaction_map[new Scattering(scattering_matrix,ngroups)] = sigma_s;

	/* ---- Fission cross section */
	vector<double> nu(ngroups);
	vector<double> nu_sigma_f = constant["nu_sigma_f"];
	for(size_t i = 0 ; i < ngroups ; ++i)
		nu[i] = nu_sigma_f[i] / sigma_f[i];
	reaction_map[new Fission(nu,constant["chi"])] = sigma_f;

	for(size_t i = 0 ; i < ngroups ; ++i)
		mfp[i] = 1.0 / (sigma_a[i] + sigma_s[i]);

	/* Setup the reaction sampler */
	reaction_sampler = new Sampler<Reaction*>(reaction_map);
}

void MacroXs::print(std::ostream& out) const {/* */}

MacroXs::~MacroXs() {
	vector<Reaction*> reactions = reaction_sampler->getReactions();
	purgePointers(reactions);
	delete reaction_sampler;
}

vector<Material*> MacroXsFactory::createMaterials(const vector<MaterialObject*>& definitions) const {
	/* Cast macro object */
	const MacroXsObject* macro_definition = static_cast<const MacroXsObject*>(*definitions.begin());
	/* Get the number of groups (just checking a particular XS)*/
	int nelement = macro_definition->getConstant()["sigma_a"].size();

	/* Container of new materials */
	vector<Material*> materials;

	/* Push materials, and check the number of groups */
	for(vector<MaterialObject*>::const_iterator it = definitions.begin() ;  it != definitions.end() ; ++it) {
		const MacroXsObject* new_macro = static_cast<const MacroXsObject*>((*it));
		/* Check number of groups */
		int ngroups = new_macro->getConstant()["sigma_a"].size();
		if(ngroups != nelement)
			throw(Material::BadMaterialCreation((*it)->getMatid(),"You can't mix materials with different number of groups"));
		MacroXs* newMaterial = new MacroXs(new_macro,nelement);
		materials.push_back(newMaterial);
	}

	/* Return container */
	return materials;
}

} /* namespace Helios */
