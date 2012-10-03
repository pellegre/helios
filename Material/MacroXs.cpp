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

static inline void copyVector(const vector<double>& stl_v, Vector& v) {
	/* Number of elements */
	int nelement = stl_v.size();
	v = Vector(nelement);
	for(size_t i = 0 ; i < nelement ; i++)
		v(i) = stl_v[i];
}

static inline void copyMatrix(const vector<double>& stl_v, Matrix& m, int nelement) {
	/* Number of elements */
	m = Matrix(nelement,nelement);
	for(int i = 0 ; i < nelement ; i++)
		for(int j = 0 ; j < nelement ; j++)
			m(i,j) = stl_v[i * nelement + j];
}

MacroXs::MacroXs(const Material::Definition* definition, int number_groups) : Material(definition),
		sigma_a(number_groups), sigma_f(number_groups), nu_sigma_f(number_groups), chi(number_groups),
		mat_sigma_s(number_groups,number_groups), sigma_t(number_groups), sigma_s(number_groups) {
	/* Cast to a MacroXs definition */
	const MacroXs::Definition* macro_definition = dynamic_cast<const MacroXs::Definition*>(definition);
	/* Get constants */
	map<string,vector<double> > constant = macro_definition->getConstant();
	/* Get the number of groups and do some error checking */
	map<string,vector<double> >::const_iterator it_xs = constant.begin();
	int ngroup = number_groups;
	for(; it_xs != constant.end() ; ++it_xs) {
		string xs_name = (*it_xs).first;
		if(xs_name == "sigma_s") {
			int size = (*it_xs).second.size();
			if(size != ngroup * ngroup) {
				throw Material::BadMaterialCreation(matid,
						"Inconsistent number of groups in constant *" + xs_name + "*");
			}
		} else {
			int size = (*it_xs).second.size();
			if(size != ngroup) {
				throw Material::BadMaterialCreation(matid,
						"Inconsistent number of groups in constant *" + xs_name + "*");
			}
		}
	}

	/* Now copy arrays from input */
	copyVector(constant["sigma_a"],sigma_a);
	copyVector(constant["sigma_f"],sigma_f);
	copyVector(constant["nu_sigma_f"],nu_sigma_f);
	copyVector(constant["chi"],chi);
	copyMatrix(constant["sigma_s"],mat_sigma_s,ngroup);
	/* Calculate the total XS and the scattering XS */
	for(size_t i = 0 ; i < ngroup ; ++i) {
		sigma_s(i) = sum(mat_sigma_s(i,Range::all()));
	}
	sigma_t = sigma_a + sigma_s;

	/* Finally, we should modify the type of material according to the number of groups */
	type += "_" + toString(ngroup) + "groups"; /* Decorate the type of material */
}

void MacroXs::print(std::ostream& out) const {
	out << "+ sigma_a " << sigma_a;
	out << "+ sigma_f " << sigma_f;
	out << "+ nu_sigma_f " << nu_sigma_f;
	out << "+ chi " << chi;
	out << "+ sigma_s " << sigma_s;
}

} /* namespace Helios */
