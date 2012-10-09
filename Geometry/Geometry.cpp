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

#include <cstdlib>
#include <set>

#include "Surface.hpp"
#include "Cell.hpp"
#include "Universe.hpp"
#include "GeometricFeature.hpp"

#include "Geometry.hpp"

using namespace std;

namespace Helios {

static inline bool getSign(const signed int& value) {return (value > 0);}

Surface* Geometry::addSurface(const Surface* surface, const Transformation& trans) {
	/* Create the new duplicated surface */
	Surface* new_surface = trans(surface);

//	/* Check if the surface is not duplicated */
//	vector<Surface*>::iterator it_sur = surfaces.begin();
//	for(; it_sur != surfaces.end() ; ++it_sur) {
//		if(*new_surface == *(*it_sur)) {
//			if(new_surface->getUserId() != (*it_sur)->getUserId())
//				if(new_surface->getUserId() <= maxUserSurfaceId)
//					Log::warn() << "Surface " << new_surface->getUserId() << " is redundant and is eliminated from the geometry" << Log::endl;
//			delete new_surface;
//			return (*it_sur);
//		}
//	}

	/* Set internal / unique index */
	new_surface->setInternalId(surfaces.size());
	/* Update surface map */
	surface_map[new_surface->getUserId()].push_back(new_surface->getInternalId());
	/* Push the surface into the container */
	surfaces.push_back(new_surface);

	/* Return the new surface */
	return new_surface;
}

Universe* Geometry::addUniverse(const UniverseId& uni_def, const map<UniverseId,vector<Cell::Definition*> >& u_cells,
		                        const map<SurfaceId,Surface*>& user_surfaces, const Transformation& trans) {
	/* Create universe */
	Universe* new_universe = UniverseFactory::access().createUniverse(uni_def);
	/* Set internal / unique index */
	new_universe->setInternalId(universes.size());
	/* Push the universe into the container */
	universes.push_back(new_universe);
	/* Update universe map */
	universe_map[new_universe->getUserId()].push_back(new_universe->getInternalId());

	map<UniverseId,vector<Cell::Definition*> >::const_iterator it_uni_cells = u_cells.find(uni_def);
	if(it_uni_cells == u_cells.end()) return 0;

	/* Get the cell of this level */
	vector<Cell::Definition*> cell_def = (*it_uni_cells).second;

	/* Add each cell of this universe */
	vector<Cell::Definition*>::iterator it_cell = cell_def.begin();

	for(; it_cell != cell_def.end() ; ++it_cell) {

		/* Cell information */
		CellId userCellId((*it_cell)->getUserCellId());
		vector<signed int> surfacesId((*it_cell)->getSurfaceIds());

		/* Now get the surfaces and put the references inside the cell */
	    vector<Cell::SenseSurface> boundingSurfaces;
	    vector<signed int>::const_iterator it = surfacesId.begin();

	    for (;it != surfacesId.end(); ++it) {
	    	/* Get user ID */
	    	SurfaceId userSurfaceId(abs(*it));

	    	/* Get internal index */
	    	map<SurfaceId,Surface*>::const_iterator it_sur = user_surfaces.find(userSurfaceId);
	    	if(it_sur == user_surfaces.end())
	    		throw Cell::BadCellCreation(userCellId,"Surface number " + toString(userSurfaceId) + " doesn't exist.");

	    	/* New surface for this cell */
	    	Surface* new_surface = addSurface((*it_sur).second,trans);

	    	/* Get surface with sense */
	    	Cell::SenseSurface newSurface(new_surface,getSign(*it));

	    	/* Push it into the container */
	        boundingSurfaces.push_back(newSurface);
	    }

	    /* Push the surfaces with sense into the cell definition */
	    (*it_cell)->setSenseSurface(boundingSurfaces);

	    /* Now we can construct the cell */
	    Cell* new_cell = CellFactory::access().createCell((*it_cell));
		/* Set internal / unique index */
	    new_cell->setInternalId(cells.size());
		/* Update cell map */
	    cell_map[new_cell->getUserId()].push_back(new_cell->getInternalId());
	    /* Update material map */
	    mat_map[new_cell->getInternalId()] = (*it_cell)->getMatId();
	    /* Push the cell into the container */
	    cells.push_back(new_cell);
	    /* Link this cell with the new universe */
	    new_universe->addCell(new_cell);

	    /* Check if this surface is filled by another universe */
	    UniverseId fill_universe_id = (*it_cell)->getFill();
	    if(fill_universe_id) {
	    	/* Create recursively the other universes and also propagate the transformation */
	    	Universe* fill_universe = addUniverse(fill_universe_id,u_cells,user_surfaces,trans + (*it_cell)->getTransformation());
	    	if(fill_universe)
	    		new_cell->setFill(fill_universe);
	    	else
	    		throw Cell::BadCellCreation(new_cell->getUserId(),
	    				"Attempting to fill with an empty universe (fill = " + toString(fill_universe_id) + ") " );
	    }
	}

	/* Return the universe */
	return new_universe;
}

template<class T>
static void pushDefinition(GeometricDefinition* geo, std::vector<T*>& definition) {
	definition.push_back(dynamic_cast<T*>(geo));
}

void Geometry::setupGeometry(std::vector<GeometricDefinition*>& definitions) {
	std::vector<Surface::Definition*> surDefinitions;
	std::vector<Cell::Definition*> cellDefinitions;
	std::vector<GeometricFeature::Definition*> featureDefinitions;
	/* Dispatch each definition to the corresponding container */
	vector<GeometricDefinition*>::const_iterator it_def = definitions.begin();

	for(; it_def != definitions.end() ; ++it_def) {
		switch((*it_def)->getType()) {
		case GeometricDefinition::CELL:
			pushDefinition(*it_def,cellDefinitions);
			break;
		case GeometricDefinition::SURFACE:
			pushDefinition(*it_def,surDefinitions);
			break;
		case GeometricDefinition::FEATURE:
			pushDefinition(*it_def,featureDefinitions);
			break;
		}
	}
	setupGeometry(surDefinitions,cellDefinitions,featureDefinitions);
	definitions.clear();
}

void Geometry::setupMaterials(const MaterialContainer& materialContainer) {
	/* Iterate over each material on the map */
	map<InternalCellId, MaterialId>::const_iterator it_mat = mat_map.begin();
	for(; it_mat != mat_map.end() ; ++it_mat) {
		/* Get cell */
		Cell* cell = cells[(*it_mat).first];
		/* Get material ID */
		MaterialId matId = (*it_mat).second;
		if(matId != Material::NONE && matId != Material::VOID) {
			try {
				cell->setMaterial(materialContainer.getMaterial(matId));
			} catch (std::exception& error) {
				throw Cell::BadCellCreation(cell->getUserId(),error.what());
			}

		} else if (matId == Material::NONE) {
			/* No material in this cell, we should check if the cell is filled with something */
			if(!cell->getFill())
				throw Cell::BadCellCreation(cell->getUserId(),"The cell is not filled with a material or a universe");
		}
	}
}

void Geometry::setupGeometry(std::vector<Surface::Definition*>& surDefinitions,
                             std::vector<Cell::Definition*>& cellDefinitions,
                             std::vector<GeometricFeature::Definition*>& featureDefinitions) {
	if(featureDefinitions.size() != 0) {
		/* Get max ID of user cells and surfaces */
		maxUserSurfaceId = surDefinitions[0]->getUserSurfaceId();
		for(vector<Surface::Definition*>::const_iterator it = surDefinitions.begin() ; it != surDefinitions.end() ; ++it) {
			SurfaceId newSurfaceId = (*it)->getUserSurfaceId();
			if (newSurfaceId > maxUserSurfaceId) maxUserSurfaceId = newSurfaceId;
		}
		maxUserCellId = cellDefinitions[0]->getUserCellId();
		for(vector<Cell::Definition*>::const_iterator it = cellDefinitions.begin() ; it != cellDefinitions.end() ; ++it) {
			CellId newCellId = (*it)->getUserCellId();
			if (newCellId > maxUserCellId) maxUserCellId = newCellId;
		}

		pair<CellId,SurfaceId> maxIds(maxUserCellId,maxUserSurfaceId);
		/* Now lets move on into the lattices */
		for(vector<GeometricFeature::Definition*>::const_iterator it = featureDefinitions.begin() ; it != featureDefinitions.end() ; ++it) {
			/* Create a lattice factory */
			GeometricFeature* feature = FeatureFactory::access().createFeature((*it),maxIds);
			maxIds = feature->createFeature((*it),surDefinitions,cellDefinitions);
			delete feature;
		}
	}

	/* First we add all the surfaces defined by the user. Ultimately, we'll have to clone and transform this ones */
	map<SurfaceId,Surface*> user_surfaces;
	vector<Surface::Definition*>::const_iterator it_sur = surDefinitions.begin();
	for(; it_sur != surDefinitions.end() ; ++it_sur) {
		/* Surface information */
		SurfaceId userSurfaceId((*it_sur)->getUserSurfaceId());

		/* Check duplicated IDs */
		map<SurfaceId,Surface*>::const_iterator it_id = user_surfaces.find(userSurfaceId);
		if(it_id != user_surfaces.end())
			throw Surface::BadSurfaceCreation(userSurfaceId,"Duplicated id");

		/* Create surface */
		Surface* new_surface = SurfaceFactory::access().createSurface((*it_sur));
		/* Update surface map */
		user_surfaces[new_surface->getUserId()] = new_surface;
	}

	/* Check for duplicated cells */
	set<CellId> user_cell_ids;
	vector<Cell::Definition*>::const_iterator it_cell = cellDefinitions.begin();
	for(; it_cell != cellDefinitions.end() ; ++it_cell) {
		CellId userCellId = (*it_cell)->getUserCellId();
		/* Check duplicated IDs */
		set<CellId>::const_iterator it_id = user_cell_ids.find(userCellId);
		if(it_id != user_cell_ids .end())
			throw Cell::BadCellCreation(userCellId,"Duplicated id");
		/* Check other stuff */
		if((*it_cell)->getFill() && ((*it_cell)->getFill() == (*it_cell)->getUniverse()))
			throw Cell::BadCellCreation(userCellId,"What are you trying to do? You can't fill a cell with the same universe in which is contained");

		user_cell_ids.insert(userCellId);
	}

	/* Map cell with universes */
	map<UniverseId,vector<Cell::Definition*> > u_cells;  /* Universe definition */
	for(it_cell = cellDefinitions.begin() ; it_cell != cellDefinitions.end() ; ++it_cell) {
		UniverseId universe = (*it_cell)->getUniverse();
		u_cells[universe].push_back(*it_cell);
	}

	addUniverse((*u_cells.begin()).first,u_cells,user_surfaces);

	/* Clean surfaces */
	map<SurfaceId,Surface*>::iterator it_user = user_surfaces.begin();
	for(; it_user != user_surfaces.end() ; ++it_user)
		delete (*it_user).second;

	/* Clean definitions, we don't need this anymore */
	purgePointers(surDefinitions);
	purgePointers(cellDefinitions);
	purgePointers(featureDefinitions);
}

void Geometry::printGeo(std::ostream& out) const {
	vector<Universe*>::const_iterator it_uni = universes.begin();
	for(; it_uni != universes.end() ; it_uni++) {
		out << "---- universe = " << (*it_uni)->getUserId() << endl;
		out << *(*it_uni);
	}
}

Geometry::~Geometry() {
	purgePointers(surfaces);
	purgePointers(cells);
	purgePointers(universes);
}

} /* namespace Helios */
