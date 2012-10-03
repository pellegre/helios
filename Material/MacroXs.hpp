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

#ifndef MACROXS_HPP_
#define MACROXS_HPP_

#include <map>

#include "Material.hpp"

namespace Helios {

class MacroXs: public Helios::Material {

	/* Number of groups */
	int ngroups;
	/* Absorption cross section */
	Vector sigma_a;
	/* Total cross section */
	Vector sigma_t;
	/* Fission cross section */
	Vector sigma_f;
	/* NU-fission cross section */
	Vector nu_sigma_f;
	/* Fission spectrum */
	Vector chi;
	/* Scattering matrix and total */
	Matrix mat_sigma_s;
	Vector sigma_s;

public:

	/* Definition of a macroscopic cross section */
	class Definition : public Material::Definition {
		/* Map of macroscopic XS name to a vector of group constant */
		std::map<std::string,std::vector<double> > constant;
	public:
		Definition(const std::string& type, const MaterialId& matid,
				   std::map<std::string,std::vector<double> >& constant) :
				   Material::Definition(type,matid), constant(constant) {/* */}

		std::map<std::string, std::vector<double> > getConstant() const {
			return constant;
		}
	};

	MacroXs(const Material::Definition* definition, int number_groups);

	void print(std::ostream& out) const;

	virtual ~MacroXs() {/* */};
};

} /* namespace Helios */
#endif /* MACROXS_HPP_ */
