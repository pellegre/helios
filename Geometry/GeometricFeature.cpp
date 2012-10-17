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

#include "GeometricFeature.hpp"
#include "Universe.hpp"
#include "Surfaces/PlaneNormal.hpp"

using namespace std;

namespace Helios {

FeatureFactory FeatureFactory::factory;

template<int axis>
static string getPlaneAbscissa() {
	switch(axis) {
	case xaxis :
		return PlaneNormal<yaxis>().name();
		break;
	case yaxis :
		return PlaneNormal<zaxis>().name();
		break;
	case zaxis :
		return PlaneNormal<xaxis>().name();
		break;
	}
	return "";
}

template<int axis>
static string getAbscissaSurface(const int& v) {
	switch(axis) {
	case xaxis :
		return "(," + toString(v) + ",)";
		break;
	case yaxis :
		return "(,," + toString(v) + ")";
		break;
	case zaxis :
		return "(" + toString(v) + ",,)";
		break;
	}
	return "";
}

template<int axis>
static string getPlaneOrdinate() {
	switch(axis) {
	case xaxis :
		return PlaneNormal<zaxis>().name();
		break;
	case yaxis :
		return PlaneNormal<xaxis>().name();
		break;
	case zaxis :
		return PlaneNormal<yaxis>().name();
		break;
	}
	return "";
}

template<int axis>
static string getOrdinateSurface(const int& v) {
	switch(axis) {
	case xaxis :
		return "(,," + toString(v) + ")";
		break;
	case yaxis :
		return "(" + toString(v) + ",,)";
		break;
	case zaxis :
		return "(," + toString(v) + ",)";
		break;
	}
	return "";
}

template<int axis>
static Direction getTranslation(const double& x, const double& y) {
	switch(axis) {
	case xaxis :
		return Direction(0,x,y);
		break;
	case yaxis :
		return Direction(y,0,x);
		break;
	case zaxis :
		return Direction(x,y,0);
		break;
	}
	return Direction();
}

template<int axis>
static string getLatticePosition(const int& x, const int& y) {
	switch(axis) {
	case xaxis :
		return "(0," + toString(x) + "," + toString(y) + ")";
		break;
	case yaxis :
		return "(" + toString(y) + "," + toString(x) + ",0)";
		break;
	case zaxis :
		return "(" + toString(x) + "," + toString(y) + ",0)";
		break;
	}
	return "";
}

/* ---- Lattice Factory stuff */

template<int axis>
/* Generation of a 2D lattice in plane perpendicular to axis */
static void gen2DLattice(const Lattice::Definition& new_lat,std::vector<Surface::Definition*>& sur_def,
		               std::vector<Cell::Definition*>& cell_def,SurfaceId& maxUserSurfaceId,CellId& maxUserCellId) {

	/* Get dimension and pitch */
	vector<unsigned int> dimension = new_lat.getDimension();
	vector<double> pitch = new_lat.getWidth();
	/* Get universes to fill each cell */
	vector<UniverseId> universes = new_lat.getUniverses();
	/* Get lattice id */
	UniverseId latt_id = new_lat.getUserFeatureId();

	/* Set width of lattice */
	vector<double> width(2);
	width[0] = pitch[0] * dimension[0];
	width[1] = pitch[1] * dimension[1];

	/* Get coordinates on x axis */
	double x_min = -width[0]/2;
	double x_max = width[0]/2;
	/* Get delta on x axis */
	double x_delta = width[0] / (double) dimension[0];

	/* Get coordinates on y axis */
	double y_min = -width[1]/2;
	double y_max = width[1]/2;
	/* Get delta on y axis */
	double y_delta = width[1] / (double) dimension[1];

	/* Now create "y" surfaces from left to right */
	vector<Surface::Definition*> y_surfaces;
	vector<double> y_coordinates;
	for(size_t i = 0 ; i <= dimension[1] ; i++) {
		vector<double> coeff;
		double sur_pos = y_min + (double)i * y_delta;
		coeff.push_back(sur_pos);
		if(i < dimension[1]) y_coordinates.push_back(sur_pos + y_delta/2);
		SurfaceId lattice_id = toString(latt_id)+getOrdinateSurface<axis>(i);
		Surface::Definition* new_surface =  new Surface::Definition(lattice_id,getPlaneOrdinate<axis>(),coeff);
		y_surfaces.push_back(new_surface);
		sur_def.push_back(new_surface);
	}

	/* Now create "x" surfaces from bottom to top */
	vector<Surface::Definition*> x_surfaces;
	vector<double> x_coordinates;
	for(size_t i = 0 ; i <= dimension[0] ; i++) {
		vector<double> coeff;
		double sur_pos = x_min + (double)i * x_delta;
		coeff.push_back(sur_pos);
		if(i < dimension[0]) x_coordinates.push_back(sur_pos + x_delta/2);
		SurfaceId lattice_id = toString(latt_id)+getAbscissaSurface<axis>(i);
		Surface::Definition* new_surface = new Surface::Definition(lattice_id,getPlaneAbscissa<axis>(),coeff);
		x_surfaces.push_back(new_surface);
		sur_def.push_back(new_surface);
	}

	size_t uni_count = 0;
	/* Now create each cell of the lattice, on the universe defined by the user (left to right, bottom to top) */
	for(int i = dimension[1] - 1 ; i >= 0  ; i--) {
		for(int j = 0 ; j < dimension[0]  ; j++) {
			vector<SurfaceId> surfs;
			surfs.push_back(y_surfaces[i]->getUserSurfaceId());
			surfs.push_back("-" + y_surfaces[i + 1]->getUserSurfaceId());
			surfs.push_back(x_surfaces[j]->getUserSurfaceId());
			surfs.push_back("-"+x_surfaces[j + 1]->getUserSurfaceId());

			/* Translate the cell to the lattice point */
			Transformation transf(getTranslation<axis>(x_coordinates[j],y_coordinates[i]));
			CellId lattice_id = toString(latt_id)+getLatticePosition<axis>(j,i);
			cell_def.push_back(new Cell::Definition(lattice_id,surfs,Cell::NONE,latt_id,universes[uni_count],Material::NONE,transf));

			/* Get next universe */
			uni_count++;
		}
	}
}

static map<string,Lattice::Constructor> initLatticeConstructorTable() {
	map<string,Lattice::Constructor> m;
	m["x-y"] = gen2DLattice<zaxis>;
	m["y-z"] = gen2DLattice<xaxis>;
	m["x-z"] = gen2DLattice<yaxis>;
	return m;
}

/* Initialize constructor map */
std::map<std::string,Lattice::Constructor> Lattice::constructor_table = initLatticeConstructorTable();

/* Constructor with current surfaces and cells on the geometry */
Lattice::Lattice(const GeometricFeature::Definition* definition, const std::pair<CellId,SurfaceId>& maxIds) :
	    GeometricFeature(definition,maxIds) {

	/* We know the definition is a Lattice::Definition */
	const Lattice::Definition* new_lat = dynamic_cast<const Lattice::Definition*>(definition);

	/* Get dimension and pitch */
	dimension = new_lat->getDimension();
	pitch = new_lat->getWidth();
	/* Get universes to fill each cell */
	universes = new_lat->getUniverses();
	UniverseId latt_id = new_lat->getUserFeatureId();

	/* ...and do some generic error checking */
	if(dimension.size() > 3) throw Universe::BadUniverseCreation(latt_id,"Dimension of the lattice is bigger than 3");
	if(pitch.size() > 3) throw Universe::BadUniverseCreation(latt_id,"You put more than 3 pitch values for the lattice");
	if(pitch.size() != dimension.size())
		throw Universe::BadUniverseCreation(latt_id,"Pitch and dimension arrays aren't of the same size");
	if(pitch.size() == 0)
		throw Universe::BadUniverseCreation(latt_id,"You need to put at least one value on the pitch and dimension arrays of the lattice");

	/* Check number of universes */
	size_t uni_count = 1;
	for(size_t i = 0 ; i < dimension.size() ; i++)
		uni_count *= dimension[i];

	if(uni_count != universes.size())
		throw Universe::BadUniverseCreation(latt_id,
		"Invalid number of universes in lattice (expected = " + toString(uni_count) + " ; input = " + toString(universes.size()) + ")");
};

std::pair<CellId,SurfaceId> Lattice::createFeature(const GeometricFeature::Definition* featureDefinition,
                              std::vector<Surface::Definition*>& surfaceDefinition,
		                      std::vector<Cell::Definition*>& cellDefinition) const {

	const Lattice::Definition* new_lat = dynamic_cast<const Lattice::Definition*>(featureDefinition);

	/* The lattice is a universe itself, so it can't be defined with an id of an existent universe */
	for(vector<Cell::Definition*>::const_iterator it_cell = cellDefinition.begin() ; it_cell != cellDefinition.end() ; ++it_cell) {
		if(new_lat->getUserFeatureId() == (*it_cell)->getUniverse())
			throw Universe::BadUniverseCreation(new_lat->getUserFeatureId(),"Duplicated id. You can't use the id of a existent universe to define a lattice");
	}

	/* Get type of lattice */
	string type = new_lat->getType();
	/* Get lattice constructor */
	map<string,Constructor>::const_iterator it_const = constructor_table.find(type);
	if(it_const == constructor_table.end())
		throw Universe::BadUniverseCreation(new_lat->getUserFeatureId(),"Lattice type " + type + " doesn't exist");

	std::pair<CellId,SurfaceId> newMaxIds(maxIds);
	/* Create lattice */
	(*it_const).second(*new_lat,surfaceDefinition,cellDefinition,newMaxIds.second,newMaxIds.first);
	/* Return new range of values */
	return newMaxIds;
}

} /* namespace Helios */
