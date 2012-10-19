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

#ifndef MATERIALOBJECT_HPP_
#define MATERIALOBJECT_HPP_

#include <string>

#include "../Environment/McModule.hpp"
#include "../Common/Common.hpp"

namespace Helios {

	/* Generic and base class to define all source related objects */
	class MaterialObject : public McObject {
		/* Material ID on this problem */
		MaterialId matid;
	public:
		/* Different type of geometric entities */
		MaterialObject(const std::string& type, const MaterialId& matid) : McObject("medium",type), matid(matid) {/* */};
		MaterialId getMatid() const {
			return matid;
		}
		virtual ~MaterialObject(){/* */};
	};

	/* Definition of a macroscopic cross section */
	class MacroXsObject : public MaterialObject {
		/* Map of macroscopic XS name to a vector of group constant */
		std::map<std::string,std::vector<double> > constant;
	public:
		MacroXsObject(const MaterialId& matid, std::map<std::string,std::vector<double> >& constant) :
			MaterialObject("macro-xs",matid), constant(constant) {/* */}

		std::map<std::string, std::vector<double> > getConstant() const {
			return constant;
		}
		~MacroXsObject() {/* */}
	};
}

#endif /* MATERIALOBJECT_HPP_ */
