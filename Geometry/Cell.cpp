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

#include <limits>

#include "Cell.hpp"
#include "Universe.hpp"

using namespace std;

namespace Helios {

/* Static global instance of the singleton */
CellFactory CellFactory::factory;

Cell::Cell(const CellId& cellid, std::vector<CellSurface>& surfaces, const CellInfo& flag) :
	cellid(cellid),
	surfaces(surfaces),
	flag(flag),
	fill(0),
	parent(0) {
    /* Set the new cell on surfaces neighbor container */
    vector<Cell::CellSurface>::iterator it_sur = surfaces.begin();
    for(; it_sur != surfaces.end() ; ++it_sur)
    	(*it_sur).first->addNeighborCell((*it_sur).second,this);
}

std::ostream& operator<<(std::ostream& out, const Cell& q) {
	q.print(out);
	return out;
}

void Cell::setFill(Universe* universe) {
	/* Link the universe filling this cell */
	fill = universe;
	fill->setParent(this);
}

void Cell::print(std::ostream& out) const {
	vector<Cell::CellSurface>::const_iterator it_sur = surfaces.begin();
	out << "cell = " << getUserId() << " (internal = " << getInternalId() << ")" << " ; universe = ";

	/* Print universe where this cell is */
	if(parent)
		out << parent->getUserId();
	else
		out << "0";

	/* If this cell is filled with another universe, print the universe ID */
	if(fill)
		out << " ; fill = " << fill->getUserId();
	out << endl;

	/* Print surfaces */
	while(it_sur != surfaces.end()) {
		out << "    ";
		if((*it_sur).second)
			out << "(+) " << (*(*it_sur).first);
		else
			out << "(-) " << (*(*it_sur).first);
		out << endl;
		++it_sur;
	}
}

const Cell* Cell::findCell(const Coordinate& position, const Surface* skip) const {
	vector<CellSurface>::const_iterator it;
	/* Deal with a negated cell */
    if (flag & NEGATED) {
		for (it = surfaces.begin() ; it != surfaces.end(); ++it) {
			if (it->first != skip) {
				if (it->first->sense(position) != it->second)
					if(fill) return fill->findCell(position,skip);
					else return this;
			} else {
				/* We just get out of the cell, we are outside for sure */
				if(fill) return fill->findCell(position,skip);
				else return this;
			}
		}
		/* We are inside all the surfaces */
		return 0;
    }
    else {
		for (it = surfaces.begin(); it != surfaces.end(); ++it) {
			if (it->first != skip) {
				if (it->first->sense(position) != it->second)
				/* The sense of the point isn't the same the same sense as we know this cell is defined... */
				return 0;
			}
		}
    }
    /* If we get here, we are inside the cell :-) */
	if(fill) return fill->findCell(position,skip);
	else return this;
}

void Cell::intersect(const Coordinate& position, const Direction& direction, Surface*& surface, bool& sense, double& distance) const {

    /* If we have a parent "cell" we should check first on upper levels first */
    const Cell* parent_cell = parent->getParent();
    if(parent_cell){
    	parent_cell->intersect(position,direction,surface,sense,distance);
    } else {
        surface = 0;
        sense = false;
        distance = std::numeric_limits<double>::infinity();
    }

    /* Loop over surfaces */
	vector<CellSurface>::const_iterator it;
	for (it = surfaces.begin() ; it != surfaces.end(); ++it) {
		/* Distance to the surface */
        double newDistance;
        /* Check the intersection with each surface */
        if(it->first->intersect(position,direction,it->second,newDistance)) {
            if (newDistance < distance) {
            	/* Update data */
                distance = newDistance;
                surface = it->first;
                sense = it->second;
            }
        }
	}

}

} /* namespace Helios */
