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

#ifndef LATTICEFACTORY_HPP_
#define LATTICEFACTORY_HPP_

#include <vector>
#include <ostream>
#include <string>

#include "Geometry.hpp"
#include "../Common/Common.hpp"

namespace Helios {

	/* Lattice factory class */
	class LatticeFactory {

	public:

		typedef void(*Constructor)(const Geometry::LatticeDefinition& new_lat,
								   std::vector<Geometry::SurfaceDefinition>& sur_def,
								   std::vector<Geometry::CellDefinition>& cell_def,
								   SurfaceId& maxUserSurfaceId,
								   CellId& maxUserCellId);

		/* Constructor with current surfaces and cells on the geometry */
		LatticeFactory(const SurfaceId& maxUserSurfaceId, const CellId& maxUserCellId) :
			maxUserSurfaceId(maxUserSurfaceId), maxUserCellId(maxUserCellId) {/* */};

		/* Create a lattice and put the new cells/surfaces into the containers */
		void createLattice(const Geometry::LatticeDefinition& new_lat,
						   std::vector<Geometry::SurfaceDefinition>& sur_def,
						   std::vector<Geometry::CellDefinition>& cell_def);

		virtual ~LatticeFactory() {/* */}

	private:
		/* Map of lattices types and constructors */
		static std::map<std::string, Constructor> constructor_table;

		/* Prevent construction or copy */
		LatticeFactory& operator= (const LatticeFactory& other);
		LatticeFactory(const LatticeFactory&);

		/* The factory keeps count of the user IDs to safely add more entities to the geometry */
		SurfaceId maxUserSurfaceId;
		CellId maxUserCellId;
	};

} /* namespace Helios */
#endif /* LATTICEFACTORY_HPP_ */
