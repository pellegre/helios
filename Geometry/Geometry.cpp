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

#include "Geometry.hpp"

using namespace std;

namespace Helios {

static inline bool getSign(const signed int& value) {return (value > 0);}

/* ---- Lattice Factory stuff */

static void genLattice(const Geometry::LatticeDefinition& new_lat,std::vector<Geometry::SurfaceDefinition>& sur_def,
		               std::vector<Geometry::CellDefinition>& cell_def,SurfaceId& maxUserSurfaceId,CellId& maxUserCellId) {

	/* Get dimension and pitch */
	vector<unsigned int> dimension = new_lat.getDimension();
	vector<double> pitch = new_lat.getWidth();
	/* Get universes to fill each cell */
	vector<UniverseId> universes = new_lat.getUniverses();
	/* Get lattice id */
	UniverseId latt_id = new_lat.getUserLatticeId();


	/* Set width of lattice */
	vector<double> width(2);
	width[xaxis] = pitch[xaxis] * dimension[xaxis];
	width[yaxis] = pitch[yaxis] * dimension[yaxis];

	/* Get coordinates on x axis */
	double x_min = -width[xaxis]/2;
	double x_max = width[xaxis]/2;
	/* Get delta on x axis */
	double x_delta = width[xaxis] / (double) dimension[xaxis];

	/* Get coordinates on y axis */
	double y_min = -width[yaxis]/2;
	double y_max = width[yaxis]/2;
	/* Get delta on y axis */
	double y_delta = width[yaxis] / (double) dimension[yaxis];

	/* Now create "y" surfaces from left to right */
	vector<Geometry::SurfaceDefinition> y_surfaces;
	vector<double> y_coordinates;
	for(size_t i = 0 ; i <= dimension[yaxis] ; i++) {
		vector<double> coeff;
		double sur_pos = y_min + (double)i * y_delta;
		coeff.push_back(sur_pos);
		if(i < dimension[yaxis]) y_coordinates.push_back(sur_pos + y_delta/2);
		Geometry::SurfaceDefinition new_surface = Geometry::SurfaceDefinition(++maxUserSurfaceId,"py",coeff,Surface::NONE);
		y_surfaces.push_back(new_surface);
		sur_def.push_back(new_surface);
	}

	/* Now create "x" surfaces from bottom to top */
	vector<Geometry::SurfaceDefinition> x_surfaces;
	vector<double> x_coordinates;
	for(size_t i = 0 ; i <= dimension[xaxis] ; i++) {
		vector<double> coeff;
		double sur_pos = x_min + (double)i * x_delta;
		coeff.push_back(sur_pos);
		if(i < dimension[xaxis]) x_coordinates.push_back(sur_pos + x_delta/2);
		Geometry::SurfaceDefinition new_surface = Geometry::SurfaceDefinition(++maxUserSurfaceId,"px",coeff,Surface::NONE);
		x_surfaces.push_back(new_surface);
		sur_def.push_back(new_surface);
	}

	size_t uni_count = 0;
	/* Now create each cell of the lattice, on the universe defined by the user (left to right, bottom to top) */
	for(int i = dimension[yaxis] - 1 ; i >= 0  ; i--) {
		for(int j = 0 ; j < dimension[xaxis]  ; j++) {
			vector<signed int> surfs;
			surfs.push_back(y_surfaces[i].getUserSurfaceId());
			surfs.push_back(-y_surfaces[i + 1].getUserSurfaceId());
			surfs.push_back(x_surfaces[j].getUserSurfaceId());
			surfs.push_back(-x_surfaces[j + 1].getUserSurfaceId());
			cell_def.push_back(Geometry::CellDefinition(++maxUserCellId,surfs,Cell::NONE,latt_id
					           ,universes[uni_count],Direction(x_coordinates[j],y_coordinates[i],0)));
			/* Get next universe */
			uni_count++;
		}
	}
}

static map<string,LatticeFactory::Constructor> initLatticeConstructorTable() {
	map<string,LatticeFactory::Constructor> m;
	m["xy"] = genLattice;
	return m;
}

/* Initialize constructor map */
std::map<std::string,LatticeFactory::Constructor> LatticeFactory::constructor_table = initLatticeConstructorTable();

void LatticeFactory::createLattice(const Geometry::LatticeDefinition& new_lat,vector<Geometry::SurfaceDefinition>& sur_def,
		vector<Geometry::CellDefinition>& cell_def) {
	/* Get dimension and pitch */
	vector<unsigned int> dimension = new_lat.getDimension();
	vector<double> pitch = new_lat.getWidth();
	/* Get universes to fill each cell */
	vector<UniverseId> universes = new_lat.getUniverses();
	/* Get lattice id */
	UniverseId latt_id = new_lat.getUserLatticeId();

	/* ...and do some generic error checking */
	if(dimension.size() > 3) throw Universe::BadUniverseCreation(latt_id,"Dimension of the lattice is bigger than 3");
	if(pitch.size() > 3) throw Universe::BadUniverseCreation(latt_id,"You put more than 3 pitch values for the lattice");
	if(pitch.size() != dimension.size())
		throw Universe::BadUniverseCreation(latt_id,"Pitch and dimension arrays aren't of the same size");
	if(pitch.size() == 0)
		throw Universe::BadUniverseCreation(latt_id,"You need to put at least one value on the pitch and dimension arrays of the lattice");

	/* Get type of lattice */
	string type = new_lat.getType();

	/* Get lattice constructor */
	map<string,Constructor>::const_iterator it_const = constructor_table.find(type);
	if(it_const == constructor_table.end())
		throw Universe::BadUniverseCreation(latt_id,"Lattice type " + type + " doesn't exist");

	/* Check number of universes */
	size_t uni_count = 1;
	for(size_t i = 0 ; i < dimension.size() ; i++)
		uni_count *= dimension[i];

	if(uni_count != universes.size())
		throw Universe::BadUniverseCreation(latt_id,
		"Invalid number of universes in lattice (expected = " + toString(uni_count) + " ; input = " + toString(universes.size()) + ")");

	/* Create lattice */
	(*it_const).second(new_lat,sur_def,cell_def,maxUserSurfaceId,maxUserCellId);

}

Geometry::Geometry() : maxUserSurfaceId(0), maxUserCellId(0) {/* */}

Surface* Geometry::addSurface(const Surface* surface, const Transformation& trans) {
	/* Create the new duplicated surface */
	Surface* new_surface = trans(surface);

	/* Check if the surface is not duplicated */
	vector<Surface*>::iterator it_sur = surfaces.begin();
	for(; it_sur != surfaces.end() ; ++it_sur) {
		if(*new_surface == *(*it_sur)) {
			if(new_surface->getUserId() != (*it_sur)->getUserId())
				if(new_surface->getUserId() < maxUserSurfaceId)
					Log::warn() << "Surface " << new_surface->getUserId() << " is redundant and is eliminated from the geometry" << Log::endl;
			delete new_surface;
			return (*it_sur);
		}
	}

	/* Set internal / unique index */
	new_surface->setInternalId(surfaces.size());
	/* Update surface map */
	surface_map[new_surface->getUserId()].push_back(new_surface->getInternalId());
	/* Push the surface into the container */
	surfaces.push_back(new_surface);

	/* Return the new surface */
	return new_surface;
}

Universe* Geometry::addUniverse(const UniverseId& uni_def, const map<UniverseId,vector<CellDefinition> >& u_cells,
		                        const map<SurfaceId,Surface*>& user_surfaces, const Transformation& trans) {
	/* Create universe */
	Universe* new_universe = UniverseFactory::access().createUniverse(uni_def);
	/* Set internal / unique index */
	new_universe->setInternalId(universes.size());
	/* Push the universe into the container */
	universes.push_back(new_universe);
	/* Update universe map */
	universe_map[new_universe->getUserId()].push_back(new_universe->getInternalId());

	map<UniverseId,vector<CellDefinition> >::const_iterator it_uni_cells = u_cells.find(uni_def);
	if(it_uni_cells == u_cells.end()) return 0;

	/* Get the cell of this level */
	vector<CellDefinition> cell_def = (*it_uni_cells).second;

	/* Add each cell of this universe */
	vector<CellDefinition>::const_iterator it_cell = cell_def.begin();

	for(; it_cell != cell_def.end() ; ++it_cell) {

		/* Cell information */
		CellId userCellId((*it_cell).getUserCellId());
		vector<signed int> surfacesId((*it_cell).getSurfacesId());
		const Cell::CellInfo flags((*it_cell).getFlags());

		/* Now get the surfaces and put the references inside the cell */
	    vector<Cell::CellSurface> boundingSurfaces;
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
	    	Cell::CellSurface newSurface(new_surface,getSign(*it));

	    	/* Push it into the container */
	        boundingSurfaces.push_back(newSurface);
	    }

	    /* Now we can construct the cell */
	    Cell* new_cell = CellFactory::access().createCell(userCellId,boundingSurfaces,flags);
		/* Set internal / unique index */
	    new_cell->setInternalId(cells.size());
		/* Update cell map */
	    cell_map[new_cell->getUserId()].push_back(new_cell->getInternalId());
	    /* Push the cell into the container */
	    cells.push_back(new_cell);
	    /* Link this cell with the new universe */
	    new_universe->addCell(new_cell);

	    /* Check if this surface is filled by another universe */
	    UniverseId fill_universe_id = (*it_cell).getFill();
	    if(fill_universe_id) {
	    	/* Create recursively the other universes and also propagate the transformation */
	    	Universe* fill_universe = addUniverse(fill_universe_id,u_cells,user_surfaces,trans + Transformation((*it_cell).getTranslation()));
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

void Geometry::addLattice(vector<SurfaceDefinition>& sur_def, vector<CellDefinition>& cell_def, vector<LatticeDefinition>& lat_def)  {
	if(lat_def.size() == 0) return;

	/* Get max ID of user cells and surfaces */
	maxUserSurfaceId = sur_def[0].getUserSurfaceId();
	for(vector<SurfaceDefinition>::const_iterator it = sur_def.begin() ; it != sur_def.end() ; ++it) {
		SurfaceId newSurfaceId = (*it).getUserSurfaceId();
		if (newSurfaceId > maxUserSurfaceId) maxUserSurfaceId = newSurfaceId;
	}
	maxUserCellId = cell_def[0].getUserCellId();
	for(vector<CellDefinition>::const_iterator it = cell_def.begin() ; it != cell_def.end() ; ++it) {
		CellId newCellId = (*it).getUserCellId();
		if (newCellId > maxUserCellId) maxUserCellId = newCellId;
	}

	/* Create a lattice factory */
	LatticeFactory latt_factory(maxUserSurfaceId,maxUserCellId);

	/* Now lets move on into the lattices */
	for(vector<LatticeDefinition>::const_iterator it = lat_def.begin() ; it != lat_def.end() ; ++it) {
		latt_factory.createLattice((*it),sur_def,cell_def);
	}
}

void Geometry::setupGeometry(vector<SurfaceDefinition>& sur_def, vector<CellDefinition>& cell_def, vector<LatticeDefinition>& lat_def) {
	/* First handle lattices, if there is at least one, we should add more surfaces/cells to the geometry */
	addLattice(sur_def,cell_def,lat_def);

	/* First we add all the surfaces defined by the user. Ultimately, we'll have to clone and transform this ones */
	map<SurfaceId,Surface*> user_surfaces;
	vector<SurfaceDefinition>::const_iterator it_sur = sur_def.begin();
	for(; it_sur != sur_def.end() ; ++it_sur) {
		/* Surface information */
		SurfaceId userSurfaceId((*it_sur).getUserSurfaceId());
		string type((*it_sur).getType());
		vector<double> coeffs((*it_sur).getCoeffs());
		Surface::SurfaceInfo flags = (*it_sur).getFlags();

		/* Check duplicated IDs */
		map<SurfaceId,Surface*>::const_iterator it_id = user_surfaces.find(userSurfaceId);
		if(it_id != user_surfaces.end())
			throw Surface::BadSurfaceCreation(userSurfaceId,"Duplicated id");

		/* Create surface */
		Surface* new_surface = SurfaceFactory::access().createSurface(type,userSurfaceId,coeffs,flags);
		/* Update surface map */
		user_surfaces[new_surface->getUserId()] = new_surface;
	}

	/* Check for duplicated cells */
	set<CellId> user_cell_ids;
	vector<CellDefinition>::const_iterator it_cell = cell_def.begin();
	for(; it_cell != cell_def.end() ; ++it_cell) {
		CellId userCellId = (*it_cell).getUserCellId();
		/* Check duplicated IDs */
		set<CellId>::const_iterator it_id = user_cell_ids.find(userCellId);
		if(it_id != user_cell_ids .end())
			throw Cell::BadCellCreation(userCellId,"Duplicated id");
		user_cell_ids.insert(userCellId);
	}

	/* Map cell with universes */
	map<UniverseId,vector<CellDefinition> > u_cells;  /* Universe definition */
	for(it_cell = cell_def.begin() ; it_cell != cell_def.end() ; ++it_cell) {
		UniverseId universe = (*it_cell).getUniverse();
		u_cells[universe].push_back(*it_cell);
	}

	addUniverse((*u_cells.begin()).first,u_cells,user_surfaces);

	/* Clean surfaces */
	map<SurfaceId,Surface*>::iterator it_user = user_surfaces.begin();
	for(; it_user != user_surfaces.end() ; ++it_user)
		delete (*it_user).second;
}

void Geometry::printGeo(std::ostream& out) const {
	vector<Universe*>::const_iterator it_uni = universes.begin();
	for(; it_uni != universes.end() ; it_uni++) {
		out << "---- universe = " << (*it_uni)->getUserId() << endl;
		out << *(*it_uni);
	}
}

Geometry::~Geometry() {
	vector<Surface*>::iterator it_sur = surfaces.begin();
	for(; it_sur != surfaces.end() ; ++it_sur)
		delete (*it_sur);

	vector<Cell*>::iterator it_cell = cells.begin();
	for(; it_cell != cells.end() ; ++it_cell)
		delete (*it_cell);

	vector<Universe*>::iterator it_uni = universes.begin();
	for(; it_uni != universes.end() ; ++it_uni)
		delete (*it_uni);
}

} /* namespace Helios */
