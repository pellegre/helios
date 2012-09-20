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

#include "Geometry.hpp"

using namespace std;

namespace Helios {

static inline bool getSign(const signed int& value) {return (value > 0);}

Geometry Geometry::geo;

Geometry::Geometry() {/* */}

void Geometry::addSurface(const SurfaceId& userSurfaceId, const string& type, const std::vector<double>& coeffs) {
	/* Check duplicated IDs */
	map<SurfaceId, InternalSurfaceId>::const_iterator it_id = surface_map.find(userSurfaceId);
	if(it_id != surface_map.end())
		throw Surface::BadSurfaceCreation(userSurfaceId,"Duplicated id");

	/* Create surface */
	Surface* new_surface = SurfaceFactory::access().createSurface(type,userSurfaceId,coeffs);
	/* Update surface map */
	surface_map[new_surface->getUserId()] = new_surface->getInternalId();
	/* Push the surface into the container */
	surfaces.push_back(new_surface);
}

void Geometry::addCell(const CellId& userCellId, const vector<signed int>& surfacesId, const Cell::CellInfo flags) {
	/* Check duplicated IDs */
	map<CellId, InternalCellId>::const_iterator it_id = cell_map.find(userCellId);
	if(it_id != cell_map.end())
		throw Cell::BadCellCreation(userCellId,"Duplicated id");

	/* Now get the surfaces and put the references inside the cell */
    vector<Cell::CellSurface> boundingSurfaces;

    vector<signed int>::const_iterator it = surfacesId.begin();
    for (;it != surfacesId.end(); ++it) {
    	/* Get user ID */
    	SurfaceId userSurfaceId(abs(*it));

    	/* Get internal index */
    	map<SurfaceId, InternalSurfaceId>::const_iterator it_id = surface_map.find(userSurfaceId);
    	if(it_id == surface_map.end())
    		throw Cell::BadCellCreation(userCellId,"Surface number " + toString(userSurfaceId) + " doesn't exist.");

    	/* Get surface with sense */
    	Cell::CellSurface newSurface(surfaces[(*it_id).second],getSign(*it));

    	/* Push it into the container */
        boundingSurfaces.push_back(newSurface);
    }

    /* Now we can construct the cell */
    Cell* new_cell = CellFactory::access().createCell(userCellId,boundingSurfaces,flags);
	/* Update cell map */
    cell_map[new_cell->getUserId()] = new_cell->getInternalId();
    /* Push the cell into the container */
    cells.push_back(new_cell);
}

Geometry::~Geometry() {
	vector<Surface*>::iterator it_sur = surfaces.begin();
	for(; it_sur != surfaces.end() ; ++it_sur)
		delete (*it_sur);

	vector<Cell*>::iterator it_cell = cells.begin();
	for(; it_cell != cells.end() ; ++it_cell)
		delete (*it_cell);
}

} /* namespace Helios */
