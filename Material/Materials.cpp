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

#include "Materials.hpp"
#include "MacroXs.hpp"

using namespace std;

namespace Helios {

Materials::Materials(const vector<McObject*>& matDefinitions, const McEnvironment* environment) : McModule(name(),environment) {
	/* Check number of definitions */
	if(matDefinitions.size() == 0)
		throw GeneralError("No information available for materials object");

	McObject* definition = matDefinitions[0];
	/* Get name of the first object */
	string name = definition->getObjectName();

	/* Container of material objects */
	vector<MaterialObject*> objects;

	for(vector<McObject*>::const_iterator it = matDefinitions.begin() ; it != matDefinitions.end() ; ++it) {
		/* Add the material */
		MaterialObject* newObject = static_cast<MaterialObject*>(*it);
		if(newObject->getObjectName() != name)
			throw Material::BadMaterialCreation(newObject->getMatid(),
					"You can't mix different types of materials. Choose " + name + " or " + newObject->getObjectName());
		objects.push_back(newObject);
	}

	/* Detect the type of materials on the materials. We can't have a mix of materials on a problem */
	if(name == MacroXs::name())
		/* Macroscopic cross section factory */
		factory = new MacroXsFactory;
	else
		throw Material::BadMaterialCreation(static_cast<MaterialObject*>(definition)->getMatid(),
				"Material type " + definition->getObjectName() + " is not defined");

	/* Set name of the type of material */
	material_type = name;

	/* Create the materials */
	materials = factory->createMaterials(objects);

	/* Update maps */
	for(size_t i = 0; i < materials.size() ; ++i) {
		/* Set internal / unique index */
		materials[i]->setInternalId(i);
		/* Update material map */
		material_map[materials[i]->getUserId()] = materials[i]->getInternalId();
	}
}

void Materials::printMaterials(std::ostream& out) const {
	vector<Material*>::const_iterator it_mat = materials.begin();
	for(; it_mat != materials.end() ; it_mat++) {
		out << *(*it_mat) << endl;
	}
}

Materials::~Materials() {
	/* Delete materials */
	purgePointers(materials);
	/* Delete factory */
	delete factory;
};

} /* namespace Helios */
