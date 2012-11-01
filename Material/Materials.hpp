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

#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include <map>
#include <vector>

#include "Material.hpp"
#include "MaterialObject.hpp"

namespace Helios {

	/*
	 * This class is the analogous as the "Geometry" class but for materials
	 * The main function is to collect all the materials in the problem and provide
	 * a centralized place to look for them.
	 */
	class Materials : public McModule {
		/* Material Factory */
		MaterialFactory* factory;

		/* Container of materials */
		std::vector<Material*> materials;
		/* Material type  */
		std::string material_type;

		/* Map internal index to user index */
		std::map<MaterialId, InternalMaterialId> material_map;

		/* Prevent copy */
		Materials(const Materials& geo);
		Materials& operator= (const Materials& other);

	public:
		/* ---- Module stuff */

		/* Name of this module */
		static std::string name() {return "materials"; }

		/* Get object */
		template<class Object>
		std::vector<Object*> getObject(const UserId& id) const;

		Materials(const std::vector<McObject*>& matDefinitions, const McEnvironment* environment);

		/* ---- Get information */

		size_t getMaterialNumber() const {return material_map.size();}
		const std::vector<Material*>& getMaterials() const {return materials;}
		const std::map<MaterialId, InternalMaterialId>& getMaterialMap() const {return material_map;};

		/* Exception */
		class MaterialError : public std::exception {
			std::string reason;
		public:
			MaterialError(const MaterialId& matid, const std::string& msg) {
				reason = "Cannot access to material " + toString(matid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~MaterialError() throw() {/* */};
		};

		/* Get a material from the user ID */
		Material* getMaterial(const MaterialId& materialId) const {
			std::map<MaterialId,InternalMaterialId>::const_iterator it_mat = material_map.find(materialId);
			if(it_mat == material_map.end())
				throw MaterialError(materialId,"Material does not exist");
			else
				return materials[(*it_mat).second];
		}

		/* Print a list of materials on the container */
		void printMaterials(std::ostream& out) const;

		virtual ~Materials();
	};

	template<class Object>
	std::vector<Object*> Materials::getObject(const UserId& id) const {
		/* Check if we are dealing with this kind of material */
		if(Object::name() != material_type)
			throw(MaterialError(id,"Material type *" + Object::name() + "* is not loaded into the system"));
		/* Get pointer */
		Object* object = dynamic_cast<Object*>(getMaterial(id));
		/* Construct container */
		std::vector<Object*> objContainer;
		objContainer.push_back(object);
		/* Return object */
		return objContainer;
	}

	class McEnvironment;

	/* Material Factory */
	class MaterialsFactory : public ModuleFactory {
	public:
		/* Prevent construction or copy */
		MaterialsFactory(McEnvironment* environment) : ModuleFactory(Materials::name(),environment) {/* */};
		/* Create a new material */
		McModule* create(const std::vector<McObject*>& objects) const {
			return new Materials(objects,getEnvironment());
		}
		virtual ~MaterialsFactory() {/* */}
	};

} /* namespace Helios */
#endif /* MATERIALS_HPP_ */
