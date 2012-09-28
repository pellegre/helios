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

/* Static global instance of the singleton */
SurfaceFactory SurfaceFactory::factory;

Surface::Surface(const SurfaceId& surfid) : surfid(surfid), flag(NONE) {/* */};
Surface::Surface(const SurfaceId& surfid, SurfaceInfo flag) : surfid(surfid), flag(flag) {/* */}

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

Surface* SurfaceFactory::createSurface(const string& type, const SurfaceId& surid, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags) const {
	map<string,Surface::Constructor>::const_iterator it_type = constructor_table.find(type);
	if(it_type != constructor_table.end())
		return (*it_type).second(surid,coeffs,flags);
	else
		throw Surface::BadSurfaceCreation(surid,"Surface type " + type + " is not defined");
}

void SurfaceFactory::registerSurface(const Surface& surface) {
	constructor_table[surface.name()] = surface.constructor();
}

std::ostream& operator<<(std::ostream& out, const Surface& q) {
	out << "surface = " << q.getUserId() << " (internal = " << q.getInternalId() << ")" << " ; type = " << q.name() << " : ";
	q.print(out);
	return out;
}

} /* namespace Helios */
