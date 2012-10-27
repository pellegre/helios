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

#include "Surface.hpp"
#include "Surfaces/SurfaceTypes.hpp"

#include "Cell.hpp"

using namespace std;

namespace Helios {

Surface::Surface(const SurfaceObject* definition) :
		surfid(definition->getUserSurfaceId()), flag(definition->getFlags()), int_surfid(0) {/* */}

void Surface::addNeighborCell(const bool& sense, Cell* cell) {
	if(sense)
		neighbor_pos.push_back(cell);
	else
		neighbor_neg.push_back(cell);
}

const std::vector<Cell*>& Surface::getNeighborCell(const bool& sense) const {
	if(sense)
		return neighbor_pos;
	else
		return neighbor_neg;
}

/* Cross a surface, i.e. find next cell. Of course, this should be called on a position located on the surface */
void Surface::cross(const Coordinate& position, const bool& sense, const Cell*& cell) const {
	/* Set to zero */
	cell = 0;
	const std::vector<Cell*>& neighbor = getNeighborCell(not sense);
	std::vector<Cell*>::const_iterator it_neighbor = neighbor.begin();
	for( ; it_neighbor != neighbor.end() ; ++it_neighbor) {
		cell = (*it_neighbor)->findCell(position,this);
		if(cell) break;
	}
}

bool Surface::cross(Particle& particle, const bool& sense, const Cell*& cell) const {
	/* Check reflecting surface */
	if(getFlags() & REFLECTING) {
		/* Get normal */
		Direction vnormal;
		normal(particle.pos(),vnormal);
		/* Reverse if necessary */
		if(sense == false) vnormal = -vnormal;
		/* Calculate the new direction */
		double projection = 2 * dot(particle.dir(), vnormal);
		particle.dir() = particle.dir() - projection * vnormal;
		return true;
	} else if (getFlags() & VACUUM) {
		/* Reach a boundary */
		return false;
	}

	/* Just a normal surface, cross and get new cell*/
	cross(particle.pos(),sense,cell);

	/* Now check if we reach a dead cell, i.e. outside the geometry */
	if(cell) /* God save the caller if this is not true... */ {
		if(cell->getFlag() & Cell::DEADCELL)
			return false;
	}

	/* We are inside the geometry */
	return true;
}

SurfaceFactory::SurfaceFactory() {
	/* Surface registering */
	registerSurface(PlaneNormal<xaxis>());          /* px - coeffs */
	registerSurface(PlaneNormal<yaxis>());          /* py - coeffs */
	registerSurface(PlaneNormal<zaxis>());          /* pz - coeffs */
	registerSurface(CylinderOnAxisOrigin<xaxis>()); /* cx - radius */
	registerSurface(CylinderOnAxisOrigin<yaxis>()); /* cy - radius */
	registerSurface(CylinderOnAxisOrigin<zaxis>()); /* cz - radius */
	registerSurface(CylinderOnAxis<xaxis>());       /* c/x - radius y z */
	registerSurface(CylinderOnAxis<yaxis>());       /* c/y - radius x z */
	registerSurface(CylinderOnAxis<zaxis>());       /* c/z - radius x y */
}

Surface* SurfaceFactory::createSurface(const SurfaceObject* definition) const {
	map<string,Surface::Constructor>::const_iterator it_type = constructor_table.find(definition->getType());
	if(it_type != constructor_table.end())
		return (*it_type).second(definition);
	else
		throw Surface::BadSurfaceCreation(definition->getUserSurfaceId(),"Surface type " + definition->getType() + " is not defined");
}

void SurfaceFactory::registerSurface(const Surface& surface) {
	constructor_table[surface.getName()] = surface.constructor();
}

std::ostream& operator<<(std::ostream& out, const Surface& q) {
	out << "surface = " << q.getUserId() << " (internal = " << q.getInternalId() << ")"
	    << " ; type = " << q.getName() << " ; flags = " << q.getFlags() << " : ";
	q.print(out);
	return out;
}

} /* namespace Helios */
