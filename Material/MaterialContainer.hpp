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

#ifndef MATERIALCONTAINER_HPP_
#define MATERIALCONTAINER_HPP_

#include <map>
#include <vector>

#include "Material.hpp"
#include "MacroXs.hpp"

namespace Helios {

/*
 * This class is the analogous as the "Geometry" class but for materials
 * The main function is to collect all the materials in the problem and provide
 * a centralized place to look for them.
 */
class MaterialContainer {

	/* Container of materials */
	std::vector<Material*> materials;

	/* Map internal index to user index */
	std::map<MaterialId, InternalMaterialId> material_map;

	/* Prevent copy */
	MaterialContainer(const MaterialContainer& geo);
	MaterialContainer& operator= (const MaterialContainer& other);

	/* Add a material */
	Material* addMaterial(const Material::Definition* definition);

public:
	MaterialContainer() {/* */};

	/* ---- Get information */

	size_t getMaterialNumber() const {return material_map.size();}

	/* Setup the material container */
	void setupMaterials(const std::vector<Material::Definition*>& matDefinitions);

	/* Print a list of materials on the container */
	void printMaterials(std::ostream& out) const;

	virtual ~MaterialContainer() {/* */};
};

} /* namespace Helios */
#endif /* MATERIALCONTAINER_HPP_ */
