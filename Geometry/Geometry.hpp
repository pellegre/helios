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

		Geometry(std::vector<GeometricDefinition*>& definitions) {setupGeometry(definitions);};

		/* ---- Geometry setup */

		/* This is the interface to setup the geometry of the problem */
		void setupGeometry(std::vector<GeometricDefinition*>& definitions);

		/* ---- Material information */

		/*
		 * This function connect each material on the container with the corresponding cell.
		 * The material ID for each cell was specified earlier on the constructor of the
		 * geometry. If some cell was defined with an inexistent material ID, this method will
		 * thrown a geometric error notifying that.
		 */
		void setupMaterials(const MaterialContainer& materialContainer);

		/* ---- Get information */

		const std::vector<Cell*>& getCells() const {return cells;};
		const std::vector<Surface*>& getSurfaces() const {return surfaces;};
		const std::vector<Universe*>& getUniverses() const {return universes;};

		/* Print cell with each surface of the geometry */
		void printGeo(std::ostream& out) const;

	    /* Find a cell given an arbitrary point in the problem (starting from the base universe) */
		const Cell* findCell(const Coordinate& position) const {
			/* Start with the base universe */
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

		/* Map internal index to user index */
		std::map<SurfaceId, std::vector<InternalSurfaceId> > surface_map;
		std::map<CellId, std::vector<InternalCellId> > cell_map;
		std::map<UniverseId, std::vector<InternalUniverseId> > universe_map;
		std::map<InternalCellId, MaterialId> mat_map;

		/*
		 * Max user IDs of surfaces and cells, this should be set when lattices are created (because we need to add
		 * more geometry entities into the problem)
		 */
		SurfaceId maxUserSurfaceId;
		CellId maxUserCellId;

		/* Prevent copy */
		Geometry(const Geometry& geo);
		Geometry& operator= (const Geometry& other);

		/*
		 * This is the interface to setup the geometry of the problem, when all definitions are dispatched to the corresponding type
		 */
		void setupGeometry(std::vector<Surface::Definition*>& surDefinitions,
				           std::vector<Cell::Definition*>& cellDefinitions,
				           std::vector<GeometricFeature::Definition*>& featureDefinitions);

		/* Add cell */
		Cell* addCell(const Cell::Definition* cellDefinition, const std::map<SurfaceId,Surface*>& user_surfaces);
		/* Add recursively all universe that are nested */
		Universe* addUniverse(const UniverseId& uni_def, const std::map<UniverseId,std::vector<Cell::Definition*> >& u_cells,
				              const std::map<SurfaceId,Surface*>& user_surfaces, const Transformation& trans = Transformation());

		/*
		 * Add a surface to the geometry, prior to check duplicated ones. If the surface exist (because the user
		 * set the same one but with different IDs) is silently discarded.
		 */
		Surface* addSurface(const Surface* surface, const Transformation& trans);
	};

} /* namespace Helios */
#endif /* GEOMETRY_HPP_ */
