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

#ifndef CELL_HPP_
#define CELL_HPP_

#include <iostream>
#include <fstream>
#include <vector>

#include "Surface.hpp"

namespace Helios {

	class Universe;

	class Cell {

	public:
		/* Pair of surface and sense */
		typedef std::pair<Surface*, bool> CellSurface;
		/* Friendly factory */
		friend class CellFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Cell& q);

		/* Hold extra information about the cell */
		enum CellInfo {
			NONE     = 0, /* No special cell attributes */
			DEADCELL = 1, /* Particles should be killed when entering us */
		};

		/* Exception */
		class BadCellCreation : public std::exception {
			std::string reason;
		public:
			BadCellCreation(const CellId& cellid, const std::string& msg) {
				reason = "Cannot create cell " + toString(cellid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadCellCreation() throw() {/* */};
		};

		/* Get container of bounding surfaces. */
		const std::vector<CellSurface>& getBoundingSurfaces() const { return surfaces;}

		/* Return the cell ID. */
		const CellId& getUserId() const {return cellid;}

		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalCellId& internal) {int_cellid = internal;}
		/* Return the internal ID associated with this surface. */
		const InternalCellId& getInternalId() const {return int_cellid;}

		/* Return information about this cell */
		CellInfo getFlag() const {return flag;}
		/* Set different options for the cell */
		void setFlags(CellInfo new_flag) {flag = new_flag;}

		/* Fill the cell with an universe */
		void setFill(Universe* universe);
		/* Get the universe that is filling this cell (NULL if any) */
		const Universe* getFill() const {return fill;}

		/* Set the parent universe of this cell */
		void setParent(Universe* parent_universe) {parent = parent_universe;}
		/* Get the universe where this cell is */
		const Universe* getParent() const {return parent;}

		/*
		 * Check if the cell contains the point and return a reference to the cell that the point is contained.
		 * The cell could be at other level (universe) on the geometry. A NULL pointer is returned if the point
		 * is not inside this cell.
		 * Optionally skip checking one surface if we know we've crossed it.
		 */
		virtual const Cell* findCell(const Coordinate& position, const Surface* skip = 0) const;

		/* Get the nearest surface to a point in a given direction */
		void intersect(const Coordinate& position, const Direction& direction, Surface*& surface, bool& sense, double& distance) const;

		virtual ~Cell() {/* */};

	protected:

		Cell(const CellId& cellid, std::vector<CellSurface>& surfaces, const CellInfo& flags);
		/* Prevent copy */
		Cell(const Cell& surface);
		Cell& operator= (const Cell& other);

		/* Print cell information */
		virtual void print(std::ostream& out) const;

		/* A vector of surfaces and senses that define this cell */
		std::vector<CellSurface> surfaces;
		/* Internal identification of this surface */
		InternalCellId int_cellid;
		/* Cell id choose by the user */
		CellId cellid;
		/* Other information about this cell */
		CellInfo flag;

		/* Reference to the universe that is filling this cell, NULL if any (material cell). */
		Universe* fill;
		/*
		 * Parent universe, which is the universe that contains this cell. NULL
		 * if this cell is on the base universe
		 */
		Universe* parent;

	};

	class CellFactory {

		/* Static instance of the factory */
		static CellFactory factory;

		/* Prevent construction or copy */
		CellFactory() {/* */};
		CellFactory& operator= (const CellFactory& other);
		CellFactory(const CellFactory&);
		virtual ~CellFactory() {/* */}

	public:
		/* Access the factory, reference to the static singleton */
		static CellFactory& access() {return factory;}

		/* Create a new surface */
		Cell* createCell(const CellId& cellid, std::vector<Cell::CellSurface>& surfaces,const Cell::CellInfo& flags) const {
			return new Cell(cellid,surfaces,flags);
		}

	};

	std::ostream& operator<<(std::ostream& out, const Cell& q);

} /* namespace Helios */
#endif /* CELL_HPP_ */
