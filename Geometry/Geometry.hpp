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
#include "../Common.hpp"

namespace Helios {

	class Geometry {

		/* Static instance of this class */
		static Geometry geo;

		/* Container of surfaces defined on the problem */
		std::vector<Surface*> surfaces;
		/* Container of cells defined on the problem */
		std::vector<Cell*> cells;
		/* Container of universes */
		std::vector<Universe*> universes;

		/* Map internal index to user index */
		std::map<SurfaceId, InternalSurfaceId> surface_map;
		std::map<CellId, InternalCellId> cell_map;
		std::map<UniverseId, InternalUniverseId> universe_map;

		/* Prevent creation */
		Geometry();
		/* Prevent copy */
		Geometry(const Geometry& geo);
		Geometry& operator= (const Geometry& other);

	public:

		/* ---- Geometry classes : Encapsulate all the geometry entities information */

		class SurfaceDefinition {
			SurfaceId userSurfaceId;
			std::string type;
			std::vector<double> coeffs;
			Surface::SurfaceInfo flags;
		public:
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
		public:
			CellDefinition(const CellId& userCellId, const std::vector<signed int>& surfacesId, const Cell::CellInfo& flags,
					       const UniverseId& universe, const UniverseId& fill) :
				userCellId(userCellId), surfacesId(surfacesId), flags(flags), universe(universe), fill(fill) {/* */}
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
			~CellDefinition() {/* */}
		};

		/* Access to the geometry of the problem */
		static inline Geometry& access() {return geo;}

		/* ---- Geometry setup */

		/* Add a surface */
		void addSurface(const SurfaceDefinition& sur_def);
		/* Add cell */
		void addCell(const CellDefinition& cell_def);
		/* Add a universe */
		void addUniverse(const UniverseId& uni_def);

		/* Print cell with each surface of the geometry */
		void printGeo(std::ostream& out) const;

	    /* Find a cell given an arbitrary point in the problem (starting from the base universe) */
		const Cell* findCell(const Coordinate& position) const;
		/* Using a universe identifier as a starting point */
		const Cell* findCell(const Coordinate& position, const InternalUniverseId& univid) const;

		/* Check geometry. This function throw an error and/or prints some warnings about the geometry */
		void checkGeometry() const;

		/* Clear and delete all the geometry stuff */
		virtual ~Geometry();
	};

} /* namespace Helios */
#endif /* GEOMETRY_HPP_ */
