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

#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include <vector>
#include <ostream>
#include <string>
#include <boost/tokenizer.hpp>

#include "Surface.hpp"
#include "Cell.hpp"
#include "Universe.hpp"
#include "GeometricFeature.hpp"
#include "GeometryObject.hpp"
#include "../Material/Materials.hpp"
#include "../Common/Common.hpp"

namespace Helios {

	class Geometry : public McModule {

	public:

		/* Name of this module */
		static std::string name() {return "geometry";}

		/* Constructor */
		Geometry(const std::vector<McObject*>& definitions, const Materials* materials = 0);

		/* Exception */
		class GeometryError : public std::exception {
			std::string reason;
		public:
			GeometryError(const std::string& msg) {
				reason = msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~GeometryError() throw() {/* */};
		};

		/* ---- Get information */

		/* Get user ID of an object */
		template<class Object>
		UserId getUserId(const Object* object) const;
		/* Get full path of an object */
		template<class Object>
		UserId getPath(const Object* object) const;
		/* Get references to objects from a path expression or id */
		template<class Object>
		std::vector<Object*> getObject(const UserId& id) const;

		/* Get container of universes */
		const std::vector<Universe*>& getUniverses() const {return universes;};
		/* Get container of surfaces */
		const std::vector<Surface*>& getSurfaces() const {return surfaces;};
		/* Get all cells */
		const std::vector<Cell*>& getCells() const {return cells;};

		/* Print cell with each surface of the geometry */
		void printGeo(std::ostream& out) const;

	    /* Find a cell given an arbitrary point in the problem (starting from the base universe) */
		const Cell* findCell(const Coordinate& position) const {
			/* Start with the base universe */
			return universes[0]->findCell(position);
		}

		/* Find a cell given an arbitrary point in the problem (with a pair position-cell known) */
		const Cell* findCell(const Cell* start, const Coordinate& position) const {
			const Cell* findCell = start->findCell(position);
			if(findCell)
				return findCell;
			else
				return universes[0]->findCell(position);
		}

		/* Using a universe identifier as a starting point */
		const Cell* findCell(const Coordinate& position, const InternalUniverseId& univid) const {
			/* Start with the universe provided */
			return universes[univid]->findCell(position);
		}

		/* Clear and delete all the geometry stuff */
		virtual ~Geometry();

	private:
		/* Prevent copy */
		Geometry(const Geometry& geo);
		Geometry& operator= (const Geometry& other);

		/* Factories */
		CellFactory cell_factory;
		SurfaceFactory surface_factory;
		FeatureFactory feature_factory;

		/* Template to hold maps from different object */
		class ObjectMap {
			/* This map an internal ID with the full path of a object */
			const std::map<InternalId,UserId>* path_map;
			/* This map the full path of a object with the internal ID */
			const std::map<UserId, InternalId>* reverse_map;
			/* This map the original object ID with all the internal objects IDs */
			const std::map<UserId, std::vector<InternalId> >* internal_map;
		public:
			ObjectMap() {/**/}
			ObjectMap(const std::map<InternalId,UserId>* path_map, const std::map<UserId, InternalId>* reverse_map,
					  const std::map<UserId, std::vector<InternalId> >* internal_map) :
					  path_map(path_map), reverse_map(reverse_map), internal_map(internal_map) {/* */}
			~ObjectMap() {/* */}
			const std::map<UserId, std::vector<InternalId> >& getInternalMap() const {return *internal_map;}
			const std::map<InternalId, UserId>& getPathMap() const {return *path_map;}
			const std::map<UserId, InternalId>& getReverseMap() const {return *reverse_map;}
		};

		/* Map of Object */
		std::map<std::string,ObjectMap> object_maps;

		/* Container of surfaces defined on the problem */
		std::vector<Surface*> surfaces;
		/* Container of cells defined on the problem */
		std::vector<Cell*> cells;
		/* Container of universes */
		std::vector<Universe*> universes;

		/* ----- Map surfaces */

		/* This map an internal ID with the full path of a surface */
		std::map<InternalSurfaceId,SurfaceId> surface_path_map;
		/* This map the full path of a surface with the internal ID */
		std::map<SurfaceId, InternalSurfaceId> surface_reverse_map;
		/* This map the original surface ID with all the internal surfaces IDs */
		std::map<SurfaceId, std::vector<InternalSurfaceId> > surface_internal_map;

		/* ----- Map cells */

		/* This map an internal ID with the full path of a cell */
		std::map<InternalCellId,CellId> cell_path_map;
		/* This map the full path of a surface with the internal ID */
		std::map<CellId, InternalCellId> cell_reverse_map;
		/* This map the original cell ID with all the internal cells IDs */
		std::map<CellId, std::vector<InternalCellId> > cell_internal_map;

		/* ----- Map universes */

		/* This map the original universe ID with all the internal universes IDs */
		std::map<UniverseId, std::vector<InternalUniverseId> > universe_map;

		/* Map of cell to materials IDs */
		std::map<InternalCellId, MaterialId> material_map;

		/* Get container of objects given the INTERNAL cells id */
		template<class Object>
		std::vector<Object*> getContainer(const std::vector<InternalId>& internal_ids) const;

		/* Parent Cell class (auxiliary, to encapsulate information about the parent cell of an universe) */
		class ParentCell {
			Transformation transformation;
			std::vector<Surface*> parent_surfaces;
			std::string id;
		public:
			ParentCell() : transformation(), parent_surfaces(), id() {/* */}
			ParentCell(const Transformation& transformation, const std::vector<Surface*>& parent_surfaces, const std::string& id) :
				transformation(transformation), parent_surfaces(parent_surfaces), id(id) {/* */}
			const std::string& getId() const {return id;}
			void setId(std::string id) {this->id = id;}
			const std::vector<Surface*>& getSurfaces() const {return parent_surfaces;}
			void setSurfaces(std::vector<Surface*> surfaces) {this->parent_surfaces = surfaces;}
			const Transformation& getTransformation() const {return transformation;}
			void setTransformation(Transformation transformation) {this->transformation = transformation;}
			~ParentCell() {/* */}
		};

		/* Add recursively all universe that are nested */
		Universe* addUniverse(const UniverseId& uni_def, const std::map<UniverseId,std::vector<CellObject*> >& u_cells,
				              const std::map<SurfaceId,Surface*>& user_surfaces, const ParentCell& parent_cell = ParentCell());

		/* Add a surface to the geometry, prior to check duplicated ones. */
		Surface* addSurface(const Surface* surface, const ParentCell& parent_cell, const std::string& surf_id);

		/* ---- Material information */

		/*
		 * This function connect each material on the container with the corresponding cell.
		 * The material ID for each cell was specified earlier on the constructor of the
		 * geometry. If some cell was defined with an inexistent material ID, this method will
		 * thrown a geometric error notifying that.
		 */
		void setupMaterials(const Materials& materials);
	};

	/* Get containers of cells */
	template<>
	std::vector<Cell*> Geometry::getContainer<Cell>(const std::vector<InternalId>& internal_ids) const;
	/* Get containers of surfaces */
	template<>
	std::vector<Surface*> Geometry::getContainer<Surface>(const std::vector<InternalId>& internal_ids) const;

	template<class Object>
	UserId Geometry::getPath(const Object* object) const {
		/* Get the internal ID */
		InternalId internal = object->getInternalId();
		std::map<InternalId,UserId> path_map = object_maps.find(Object::name())->second.getPathMap();
		std::map<InternalId,UserId>::const_iterator it = path_map.find(internal);
		/* This is the full path of this cell */
		return (*it).second;
	}

	template<class Object>
	UserId Geometry::getUserId(const Object* object) const {
		/* This is the full path of this cell */
		UserId full_path = getPath(object);
		/* Get the original ID of the cell */
		boost::char_separator<char> sep("< ");
		boost::tokenizer<boost::char_separator<char> > tok(full_path,sep);
		return *tok.begin();
	}

	template<class Object>
	std::vector<Object*> Geometry::getObject(const UserId& orig_id) const {
		std::string id(orig_id);
		id.erase(std::remove_if(id.begin(), id.end(),::isspace), id.end());
		/* Get maps */
		std::map<UserId,InternalId> reverse_map = object_maps.find(Object::name())->second.getReverseMap();
		std::map<UserId,std::vector<InternalId> > internal_map = object_maps.find(Object::name())->second.getInternalMap();
		/* Detect if is a full path (only one cell) or a group of cells */
		if(id.find("<") != std::string::npos) {
			/* One specific cell */
			std::map<UserId,InternalId>::iterator it = reverse_map.find(id);
			if(it != reverse_map.end()) {
				std::vector<InternalId> internal_ids;
				internal_ids.push_back(it->second);
				return getContainer<Object>(internal_ids);
			}
			else
				throw GeometryError("Could not find any " + Object::name() + " on path " + id);
		}
		else {
			/* Group of objects (or a object on top level) */
			std::map<UserId,std::vector<InternalId> >::const_iterator it = internal_map.find(id);
			if(it != internal_map.end()) {
				std::vector<InternalId> internal_ids = (*it).second;
				return getContainer<Object>(internal_ids);
			} else
				throw GeometryError(Object::name() + " " + id + " does not exist");
		}
	}

	class McEnvironment;

	/* Material Factory */
	class GeometryFactory : public ModuleFactory {
	public:
		/* Prevent construction or copy */
		GeometryFactory(McEnvironment* environment) : ModuleFactory(Geometry::name(),environment) {/* */};
		/* Create a new material */
		McModule* create(const std::vector<McObject*>& objects) const;
		virtual ~GeometryFactory() {/* */}
	};

} /* namespace Helios */
#endif /* GEOMETRY_HPP_ */
