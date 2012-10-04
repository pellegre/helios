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

#include "MaterialContainer.hpp"

using namespace std;

namespace Helios {

void MaterialContainer::setupMaterials(const vector<Material::Definition*>& matDefinitions) {
	string new_type, old_type;
	for(vector<Material::Definition*>::const_iterator it = matDefinitions.begin() ; it != matDefinitions.end() ; ++it) {
		/* Add the material */
		Material* new_mat = addMaterial((*it));
		new_type = new_mat->getType();
		if(it != matDefinitions.begin() && new_type != old_type)
			throw Material::BadMaterialCreation(new_mat->getUserId(),
					"You can't mix different types of material representations. " + old_type + " is different from " + new_type);
		old_type = new_type;
	}
}

Material* MaterialContainer::addMaterial(const Material::Definition* definition) {
	/* Check if the material is not duplicated */
	map<MaterialId, InternalMaterialId>::const_iterator it_mat = material_map.find(definition->getMatid());
	if(it_mat != material_map.end())
			throw(Material::BadMaterialCreation(definition->getMatid(),"Duplicated id "));

	/* Create the new material */
	Material* new_material = MaterialFactory::access().createMaterial(definition);
	/* Set internal / unique index */
	new_material->setInternalId(materials.size());
	/* Update material map */
	material_map[new_material->getUserId()] = new_material->getInternalId();
	/* Push the material into the container */
	materials.push_back(new_material);

	/* Return the new material */
	return new_material;
}

void MaterialContainer::printMaterials(std::ostream& out) const {
	vector<Material*>::const_iterator it_mat = materials.begin();
	for(; it_mat != materials.end() ; it_mat++) {
		out << *(*it_mat) << endl;
	}
}

} /* namespace Helios */
