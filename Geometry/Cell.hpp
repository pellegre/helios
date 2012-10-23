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
#include "../Material/Material.hpp"
#include "Transformation.hpp"
#include "GeometryObject.hpp"

namespace Helios {

	class Universe;
	class CellObject;

	class Cell {

	public:
		/* Object name */
		static std::string name() {return "cell";}

		/* Pair of surface and sense */
		typedef std::pair<Surface*, bool> SenseSurface;
		/* Friendly factory */
		friend class CellFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Cell& q);

		/* Hold extra information about the cell */
		enum CellInfo {
			NONE     = 0, /* No special cell attributes */
			DEADCELL = 1  /* Particles should be killed when entering us */
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

		/* Return the internal ID associated with this cell. */
		const CellId& getUserId() const {return user_id;}

		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalCellId& internal) {internal_id = internal;}
		/* Return the internal ID associated with this cell. */
		const InternalCellId& getInternalId() const {return internal_id;}

		/* Return information about this cell */
		CellInfo getFlag() const {return flag;}
		/* Set different options for the cell */
		void setFlags(CellInfo new_flag) {flag = new_flag;}

		/* Fill the cell with an universe */
		void setFill(Universe* universe);
		/* Get the universe that is filling this cell (NULL if any) */
		const Universe* getFill() const {return fill;}

		/* Fill the cell with a material */
		void setMaterial(Material* cell_mat) {material = cell_mat;};
		/* Get the material that is filling this cell (NULL if any) */
		const Material* getMaterial() const {return material;}

		/* Set the parent universe of this cell */
		void setParent(Universe* parent_universe) {parent = parent_universe;}
		/* Get the universe where this cell is */
		const Universe* getParent() const {return parent;}

		/*
		 * Check if the cell contains the point and return a reference to the cell that the point is contained.
		 * The cell could be at other level (universe) on the geometry (a recursive search is done).
		 * A NULL pointer is returned if the point is not inside this cell.
		 * Optionally skip checking one surface if we know we've crossed it.
		 */
		const Cell* findCell(const Coordinate& position, const Surface* skip = 0) const;

		/*
		 * Check if the cell contains the point.
		 * Optionally skip checking one surface if we know we've crossed it.
		 */
		bool isInside(const Coordinate& position, const Surface* skip = 0) const;

		/* Get the nearest surface to a point in a given direction */
		void intersect(const Coordinate& position, const Direction& direction, Surface*& surface, bool& sense, double& distance) const;

		virtual ~Cell() {/* */};

	protected:

		Cell(const CellObject* definition, const std::vector<SenseSurface>& surfaces);
		/* Prevent copy */
		Cell(const Cell& cell);
		Cell& operator= (const Cell& other);

		/* A vector of surfaces and senses that define this cell */
		std::vector<SenseSurface> surfaces;
		/* Other information about this cell */
		CellInfo flag;
		/* Reference to the universe that is filling this cell, NULL if any (material cell). */
		Universe* fill;
		/* Material filling this cell (could be null, when the material is void or the cell is filled by an universe) */
		Material* material;
		/*
		 * Parent universe, which is the universe that contains this cell. A cell
		 * always has a parent (even in the base universe)
		 */
		Universe* parent;
		/* Internal identification of this cell */
		InternalCellId internal_id;
		/* cCell id choose by the user */
		CellId user_id;
	};

	/* Output surface information */
	std::ostream& operator<<(std::ostream& out, const Cell& q);

	class CellObject : public GeometryObject {
		CellId user_cell_id;
		Cell::CellInfo flags;
		UniverseId universe;
		UniverseId fill;
		MaterialId matId;
		Transformation transformation;
		std::string surfaces_expression;   /* IDs of the surfaces */
		friend class Cell;
	public:

		CellObject() : GeometryObject(Cell::name()) {/* */}
		CellObject(const CellId& userCellId, const std::string& surfaces_expression, const Cell::CellInfo flags,
				   const UniverseId& universe, const UniverseId& fill,const MaterialId& matId, const Transformation& transformation) :
				   GeometryObject(Cell::name()), user_cell_id(userCellId), surfaces_expression(surfaces_expression),
				   flags(flags),universe(universe), fill(fill), matId(matId), transformation(transformation) {/* */}
		Cell::CellInfo getFlags() const {return flags;}
		CellId getUserCellId() const {return user_cell_id;}
		UniverseId getUniverse() const {return universe;}
		UniverseId getFill() const {return fill;}
		MaterialId getMatId() const {return matId;}
		Transformation getTransformation() const {return transformation;}
		/* Getting surfaces */
		std::string getSurfacesExpression() const {return surfaces_expression;}
		~CellObject() {/* */}
	};

	class CellFactory {
	public:
		/*
		 * Given a surface expression, the functions returns the unique surfaces IDs on that expression.
		 * This function is called by the Geometry class to know which surfaces should create when constructing
		 * a given cell
		 */
		static std::vector<SurfaceId> getSurfacesIds(const std::string& surface_expresion);

		/* Prevent construction or copy */
		CellFactory() {/* */};
		/* Create a new surface */
		Cell* createCell(const CellObject* definition, std::map<SurfaceId,Surface*>& cell_surfaces) const;
		virtual ~CellFactory() {/* */}
	};

} /* namespace Helios */
#endif /* CELL_HPP_ */
