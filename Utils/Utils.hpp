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

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <random/uniform.h>

#include "../Log/Log.hpp"
#include "../Geometry/Geometry.hpp"

/* Search keywords inside a file (and gte the line with the best match) */
size_t seachKeyWords(const std::string& filename, std::vector<std::string> search_keys);

/* Simple plotter, just for debugging */
void plot(const Helios::Geometry& geo, double xmin, double xmax, double ymin, double ymax, const std::string& filename);

/* ---- Debugging and testing functions */

/* Return a random number between 0.0 and 1.0 */
double randomNumber() {
	static Uniform<double> uniform;
	return uniform.random();
}

/* Return a random direction on space */
Helios::Direction randomDirection() {
	return Helios::Direction(randomNumber(),randomNumber(),randomNumber());
}

/*
 * Transport a particle from a start point with a fixed direction until a dead cell is found.
 * The function returns two vectors :
 * - cells    : Container of user defined cells IDs that are crossed by the particle.
 * - surfaces : Container of user defined surfaces IDs that are crossed by the particle.
 */
void transport(const Helios::Geometry& geometry, const Helios::Coordinate& start_pos, const Helios::Direction& direction,
		       std::vector<Helios::CellId>& cells, std::vector<Helios::SurfaceId>& surfaces);

/*
 * Move a coordinate from surface to surface and check the surface equation. This is done a fixed number times
 * until the coordinate reach a dead cell. Returns the MAX evaluation of a surface equation.
 */
double randomTransport(const Helios::Geometry& geometry, const Helios::Coordinate& start_pos);

#endif /* UTILS_HPP_ */
