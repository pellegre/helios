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

#include "../Common/Common.hpp"

namespace Helios {

	class Universe;
	class Surface;

	class Transformation {

		/* A translation transformation */
		Direction translation;
		/* A rotation (degrees around each of the 3 axis) */
		Direction rotation;

	public:
		Transformation(const Direction& translation = Direction(0,0,0), const Direction& rotation = Direction(0,0,0))
						: translation(translation), rotation(rotation) {/* */}

		/* Returns a new instance of a cloned transformed surface */
		Surface* operator()(const Surface* surface) const;

		/* Sum transformations */
		const Transformation operator+(const Transformation& right) const {
			return Transformation(right.translation + translation, right.rotation + rotation);
		}

		~Transformation() {/* */}
	};

	class Cell {

	public:
		/* Pair of surface and sense */
		typedef std::pair<Surface*, bool> SenseSurface;
		/* Friendly factory */
		friend class CellFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Cell& q);

		/* Hold extra information about the cell */
		enum CellInfo {
			NONE     = 0, /* No special cell attributes */
			DEADCELL = 2  /* Particles should be killed when entering us */
		};

		class Definition {
			CellId userCellId;
			Cell::CellInfo flags;
			UniverseId universe;
			UniverseId fill;
			Transformation transformation;

			/* Handling surfaces */
			std::vector<signed int> surfacesIds;   /* IDs of the surfaces */
			std::vector<SenseSurface> surfacesPtrs; /* Pair of sense and pointer to a set of constructed surfaces */
		public:

			Definition() {/* */}
			Definition(const CellId& userCellId, const std::vector<signed int>& surfacesIds, const Cell::CellInfo flags,
					   const UniverseId& universe, const UniverseId& fill, const Transformation& transformation) :
				       userCellId(userCellId), surfacesIds(surfacesIds), flags(flags),
				       universe(universe), fill(fill), transformation(transformation) {/* */}
			Cell::CellInfo getFlags() const {
				return flags;
			}
			CellId getUserCellId() const {
				return userCellId;
			}
			UniverseId getUniverse() const {
				return universe;
			}
			UniverseId getFill() const {
				return fill;
			}
			Transformation getTransformation() const {
				return transformation;
			}

			/* Setting surfaces */
			void setSenseSurface(const std::vector<SenseSurface>& ptrs) {
				surfacesPtrs = ptrs;
			}
			std::vector<SenseSurface> getSenseSurface() const {
				return surfacesPtrs;
			}

			/* Getting surfaces */
			std::vector<signed int> getSurfaceIds() const {
				return surfacesIds;
			}

			~Definition() {/* */}
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
		const std::vector<SenseSurface>& getBoundingSurfaces() const { return surfaces;}

		/* Return the cell ID. */
		const CellId& getUserId() const {return cellid;}

		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalCellId& internal) {int_cellid = internal;}
		/* Return the internal ID associated with this cell. */
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

		Cell(const Definition* definition);
		/* Prevent copy */
		Cell(const Cell& cell);
		Cell& operator= (const Cell& other);

		/* Print cell information */
		virtual void print(std::ostream& out) const;

		/* Cell id choose by the user */
		CellId cellid;
		/* A vector of surfaces and senses that define this cell */
		std::vector<SenseSurface> surfaces;
		/* Other information about this cell */
		CellInfo flag;
		/* Reference to the universe that is filling this cell, NULL if any (material cell). */
		Universe* fill;
		/*
		 * Parent universe, which is the universe that contains this cell. NULL
		 * if this cell is on the base universe
		 */
		Universe* parent;
		/* Internal identification of this surface */
		InternalCellId int_cellid;

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
		Cell* createCell(const Cell::Definition* definition) const {
			return new Cell(definition);
		}

	};

	std::ostream& operator<<(std::ostream& out, const Cell& q);

} /* namespace Helios */
#endif /* CELL_HPP_ */
