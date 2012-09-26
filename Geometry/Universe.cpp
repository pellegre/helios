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

#include "Universe.hpp"

using namespace std;

namespace Helios {

/* Initialize global universe counter */
size_t Universe::counter = 0;

/* Static global instance of the singleton */
UniverseFactory UniverseFactory::factory;

Universe::Universe(const UniverseId& univid) : univid(univid) {
	/* Set internal ID */
	int_univid = counter;
	/* Increment counter */
	counter++;
}

const Cell* Universe::findCell(const Coordinate& position, const Surface* skip) const {
	/* loop through all cells in problem */
	for (vector<Cell*>::const_iterator it_cell = cells.begin(); it_cell != cells.end(); ++it_cell) {
		const Cell* in_cell = (*it_cell)->findCell(position,skip);
		if (in_cell) return in_cell;
	}
	return 0;
}

std::ostream& operator<<(std::ostream& out, const Universe& q) {
	vector<Cell*>::const_iterator it_cell = q.cells.begin();
	for(; it_cell != q.cells.end() ; it_cell++)
		out << *(*it_cell);
	return out;
}

} /* namespace Helios */
