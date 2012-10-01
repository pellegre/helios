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

#include "../Common/Common.hpp"

namespace Helios {

	class Geometry {

	public:

		/* Prevent creation */
		Geometry();

		/* ---- Geometry classes : Encapsulate all the geometry entities information */

		class SurfaceDefinition {
			SurfaceId userSurfaceId;
			std::string type;
			std::vector<double> coeffs;
			Surface::SurfaceInfo flags;
		public:
			SurfaceDefinition() {/* */}
			SurfaceDefinition(const SurfaceId& userSurfaceId, const std::string& type, const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags = Surface::NONE) :
				userSurfaceId(userSurfaceId), type(type), coeffs(coeffs), flags(flags) {/* */}
			std::vector<double> getCoeffs() const {
				return coeffs;
			}
			std::string getType() const {
				return type;
			}
			SurfaceId getUserSurfaceId() const {
				return userSurfaceId;
			}
			Surface::SurfaceInfo getFlags() const {
				return flags;
			}
			~SurfaceDefinition() {/* */}
		};

		class CellDefinition {
			CellId userCellId;
			std::vector<signed int> surfacesId;
			Cell::CellInfo flags;
			UniverseId universe;
			UniverseId fill;
			Direction translation;
		public:
			CellDefinition() {/* */}
			CellDefinition(const CellId& userCellId, const std::vector<signed int>& surfacesId, const Cell::CellInfo flags,
					       const UniverseId& universe, const UniverseId& fill, const Direction& translation) :
				userCellId(userCellId), surfacesId(surfacesId), flags(flags), universe(universe), fill(fill), translation(translation) {/* */}
			Cell::CellInfo getFlags() const {
				return flags;
			}
			std::vector<signed int> getSurfacesId() const {
				return surfacesId;
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
			Direction getTranslation() const {
				return translation;
			}
			~CellDefinition() {/* */}
		};

		class LatticeDefinition {

		public:

			LatticeDefinition() {/* */}
			LatticeDefinition(const UniverseId& userLatticeId, const std::string& type, const std::vector<unsigned int>& dimension,
					          const std::vector<double>& width, const std::vector<UniverseId>& universes) :
					          userLatticeId(userLatticeId), type(type), dimension(dimension), width(width), universes(universes) {/* */}

			std::vector<unsigned int> getDimension() const {
				return dimension;
			}

			std::string getType() const {
				return type;
			}

			std::vector<UniverseId> getUniverses() const {
				return universes;
			}

			UniverseId getUserLatticeId() const {
				return userLatticeId;
			}

			std::vector<double> getWidth() const {
				return width;
			}

			~LatticeDefinition() {/* */}

		private:
			UniverseId userLatticeId;
			std::string type;
			std::vector<unsigned int> dimension;
			std::vector<double> width;
			std::vector<UniverseId> universes;

		};
		/* ---- Get information */

		size_t getCellNumber() const {return cells.size();}
		size_t getSurfaceNumber() const {return surfaces.size();}
		size_t getUniverseNumber() const {return universes.size();}

		/* ---- Geometry setup */

		/* This is the interface to setup the geometry of the problem */
		void setupGeometry(std::vector<SurfaceDefinition>& sur_def, std::vector<CellDefinition>& cell_def,
				           std::vector<LatticeDefinition>& lat_def);

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

		/*
		 * Max user IDs of surfaces and cells, this should be set when lattices are created (because we need to add
		 * more gemoetry entities into the problem)
		 */
		SurfaceId maxUserSurfaceId;
		CellId maxUserCellId;

		/* Prevent copy */
		Geometry(const Geometry& geo);
		Geometry& operator= (const Geometry& other);

		/* Add a surface */
		Surface* addSurface(const SurfaceDefinition& sur_def);
		/* Add cell */
		Cell* addCell(const CellDefinition& cell_def, const std::map<SurfaceId,Surface*>& user_surfaces);
		/* Add recursively all universe that are nested */
		Universe* addUniverse(const UniverseId& uni_def, const std::map<UniverseId,std::vector<CellDefinition> >& u_cells,
				              const std::map<SurfaceId,Surface*>& user_surfaces, const Transformation& trans = Transformation());

		/* Add more surfaces/cells to construct the lattice (always centered at 0.0) */
		void addLattice(std::vector<SurfaceDefinition>& sur_def, std::vector<CellDefinition>& cell_def,
				           std::vector<LatticeDefinition>& lat_def);

		/*
		 * Add a surface to the geometry, prior to check duplicated ones. If the surface exist (because the user
		 * set the same one but with different IDs) is silently discarded.
		 */
		Surface* addSurface(const Surface* surface, const Transformation& trans);
	};

} /* namespace Helios */
#endif /* GEOMETRY_HPP_ */
