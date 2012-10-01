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

#include "Utils.hpp"

using namespace std;
using namespace Helios;

void transport(const Helios::Geometry& geometry, const Helios::Coordinate& start_pos, const Helios::Direction& start_dir,
		       vector<Helios::CellId>& cells, vector<Helios::SurfaceId>& surfaces) {
	Helios::Coordinate pos(start_pos);
	/* Geometry stuff */
	const Cell* cell(geometry.findCell(pos));
	cells.push_back(cell->getUserId());
	Surface* surface(0);
	bool sense(true);
	double distance(0.0);
	while(cell) {
		/* Get next surface and distance */
		cell->intersect(pos,start_dir,surface,sense,distance);
		/* Transport the particle */
		pos = pos + distance * start_dir;
		/* Now get next cell */
		surface->cross(pos,sense,cell);
		/* Put user IDs */
		cells.push_back(cell->getUserId());
		surfaces.push_back(surface->getUserId());
		if(cell->getFlag() & Cell::DEADCELL) break;
	}
}

double randomTransport(const Helios::Geometry& geometry, const Helios::Coordinate& start_pos) {
	double max_eval = 0.0;
	Helios::Coordinate pos(start_pos);
	/* Geometry stuff */
	const Cell* cell(geometry.findCell(pos));
	Surface* surface(0);
	bool sense(true);
	double distance(0.0);
	while(cell) {
		Direction start_dir(randomDirection());
		/* Get next surface and distance */
		cell->intersect(pos,start_dir,surface,sense,distance);
		if(surface->getFlags() & Surface::VACUUM) break;
		/* Transport the particle */
		pos = pos + distance * start_dir;
		max_eval = std::max(max_eval,surface->function(pos));
		/* Now get next cell */
		surface->cross(pos,sense,cell);
		if(cell->getFlag() & Cell::DEADCELL) break;
	}
	return max_eval;
}
