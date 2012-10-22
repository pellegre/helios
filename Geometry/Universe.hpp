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

#ifndef UNIVERSE_HPP_
#define UNIVERSE_HPP_

#include <vector>

#include "Cell.hpp"
#include "Surface.hpp"

#include "../Common/Common.hpp"

namespace Helios {

	class Geometry;

	class Universe {

		friend class UniverseFactory;

		/* Internal identification of this universe */
		InternalUniverseId internal_id;
		/* A vector of cells */
		std::vector<Cell*> cells;
		/* Universe id choose by the user */
		UniverseId user_id;
		/*
		 * Parent cell : Each universe has ONLY one parent cell. If more than one cell
		 * is filled with the same universe, the universe is cloned. The base universe
		 * has a NULL parent.
		 */
		Cell* parent;

	protected:

		/* Prevent copy */
		Universe(const Universe& uni);
		Universe& operator=(const Universe& uni);

	public:

		friend std::ostream& operator<<(std::ostream& out, const Universe& q);

		/* Name of this object */
		static std::string name() {return "universe";}

		Universe(const UniverseId& univid, Cell* parent = 0);

		/* Constant to reference the base universe */
		static const UniverseId BASE;

		/* Exception */
		class BadUniverseCreation : public std::exception {
			std::string reason;
		public:
			BadUniverseCreation(const UniverseId& uniid, const std::string& msg) {
				reason = "Cannot create universe " + toString(uniid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadUniverseCreation() throw() {/* */};
		};

		/* Add a cell */
		void addCell(Cell* cell);
		/* Get cells of this universe */
		const std::vector<Cell*>& getCells() const {return cells;};

		/* Find cell inside the universe */
		const Cell* findCell(const Coordinate& position, const Surface* skip = 0) const {
			/* loop through all cells in problem */
			for (std::vector<Cell*>::const_iterator it_cell = cells.begin(); it_cell != cells.end(); ++it_cell) {
				const Cell* in_cell = (*it_cell)->findCell(position,skip);
				if (in_cell) return in_cell;
			}
			return 0;
		}

		/* Set a parent for this universe */
		void setParent(Cell* cell) {parent = cell;};
		/* Get parent cell */
		const Cell* getParent() const {return parent;}

		/* Return the user ID associated with the universe. */
		const UniverseId& getUserId() const {return user_id;}
		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalCellId& internal) {internal_id = internal;}
		/* Return the internal ID associated with the universe. */
		const InternalUniverseId& getInternalId() const {return internal_id;}

		virtual ~Universe() {/* */};
	};


} /* namespace Helios */
#endif /* UNIVERSE_HPP_ */
