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

#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include <vector>
#include <ostream>
#include <string>

#include "Surface.hpp"
#include "Cell.hpp"
#include "Universe.hpp"
#include "GeometricFeature.hpp"
#include "GeometricDefinition.hpp"
#include "../Material/MaterialContainer.hpp"
#include "../Common/Common.hpp"

namespace Helios {

	class Geometry {

	public:

		Geometry(std::vector<GeometricDefinition*>& definitions) {
			setupGeometry(definitions);
		};

		/* Exception */
		class GeometryError : public std::exception {
			std::string reason;
		public:
			GeometryError(const std::string& msg) {
				reason = msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~GeometryError() throw() {/* */};
		};

		/* ---- Material information */

		/*
		 * This function connect each material on the container with the corresponding cell.
		 * The material ID for each cell was specified earlier on the constructor of the
		 * geometry. If some cell was defined with an inexistent material ID, this method will
		 * thrown a geometric error notifying that.
		 */
		void setupMaterials(const MaterialContainer& materialContainer);

		/* ---- Get Cell information */

		/* Get user ID of a cell */
		CellId getUserId(const Cell* cell) const;
		/* Get full path of a cell */
		CellId getPath(const Cell* cell) const;
		/* Get all cells */
		const std::vector<Cell*>& getCells() const {return cells;};
		/* Get references to cells from a path expression */
		std::vector<Cell*> getCells(const std::string& path);

		/* ---- Get Surface information */

		/* Get user ID of a cell */
		SurfaceId getUserId(const Surface* surf) const;
		/* Get full path of a cell */
		SurfaceId getPath(const Surface* surf) const;
		/* Get container of surfaces */
		const std::vector<Surface*>& getSurfaces() const {return surfaces;};
		/* Get references to cells from a path expression */
		std::vector<Surface*> getSurfaces(const std::string& path);

		const std::vector<Universe*>& getUniverses() const {return universes;};

		/* Print cell with each surface of the geometry */
		void printGeo(std::ostream& out) const;

	    /* Find a cell given an arbitrary point in the problem (starting from the base universe) */
		const Cell* findCell(const Coordinate& position) const {
			/* Start with the base universe */
			return universes[0]->findCell(position);
		}

		/* Find a cell given an arbitrary point in the problem (with a pair position-cell known) */
		const Cell* findCell(const Cell* start, const Coordinate& position) const {
			const Cell* findCell = start->findCell(position);
			if(findCell)
				return findCell;
			else
				return universes[0]->findCell(position);
		}

		/* Using a universe identifier as a starting point */
		const Cell* findCell(const Coordinate& position, const InternalUniverseId& univid) const {
			/* Start with the universe provided */
			return universes[univid]->findCell(position);
		}

		/* Clear and delete all the geometry stuff */
		virtual ~Geometry();

	private:

		/* Container of surfaces defined on the problem */
		std::vector<Surface*> surfaces;
		/* Container of cells defined on the problem */
		std::vector<Cell*> cells;
		/* Container of universes */
		std::vector<Universe*> universes;

		/* ----- Map surfaces */

		/* This map an internal ID with the full path of a surface */
		std::map<InternalSurfaceId,SurfaceId> surface_path_map;
		/* This map the full path of a surface with the internal ID */
		std::map<SurfaceId, InternalSurfaceId> surface_reverse_map;
		/* This map the original cell ID with all the internal surfaces IDs */
		std::map<SurfaceId, std::vector<InternalSurfaceId> > surface_internal_map;

		/* ----- Map cells */

		/* This map an internal ID with the full path of a cell */
		std::map<InternalCellId,CellId> cell_path_map;
		/* This map the full path of a surface with the internal ID */
		std::map<CellId, InternalCellId> cell_reverse_map;
		/* This map the original cell ID with all the internal cells IDs */
		std::map<CellId, std::vector<InternalCellId> > cell_internal_map;

		std::map<UniverseId, std::vector<InternalUniverseId> > universe_map;
		std::map<InternalCellId, MaterialId> mat_map;

		/* Prevent copy */
		Geometry(const Geometry& geo);
		Geometry& operator= (const Geometry& other);

		/* ---- Geometry setup */

		/* This is the interface to setup the geometry of the problem */
		void setupGeometry(std::vector<GeometricDefinition*>& definitions);

		/*
		 * This is the interface to setup the geometry of the problem, when all definitions
		 * are dispatched to the corresponding type
		 */
		void setupGeometry(std::vector<Surface::Definition*>& surDefinitions,
				           std::vector<Cell::Definition*>& cellDefinitions,
				           std::vector<GeometricFeature::Definition*>& featureDefinitions);

		/* Add cell */
		Cell* addCell(const Cell::Definition* cellDefinition, const std::map<SurfaceId,Surface*>& user_surfaces);
		/* Add recursively all universe that are nested */
		Universe* addUniverse(const UniverseId& uni_def, const std::map<UniverseId,std::vector<Cell::Definition*> >& u_cells,
				              const std::map<SurfaceId,Surface*>& user_surfaces, const Transformation& trans = Transformation(),
				              const std::vector<Cell::SenseSurface>& parent_surfaces = std::vector<Cell::SenseSurface>(),
				              const std::string& parent_id = "");
		/* Add a surface to the geometry, prior to check duplicated ones. */
		Surface* addSurface(const Surface* surface, const Transformation& trans,const std::vector<Cell::SenseSurface>& parent_surfaces,
				            const std::string& parent_id, const std::string& surf_id);
	};

} /* namespace Helios */
#endif /* GEOMETRY_HPP_ */
