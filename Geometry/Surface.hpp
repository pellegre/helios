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

#ifndef SURFACE_HPP_
#define SURFACE_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>

#include "../Common/Common.hpp"
#include "Cell.hpp"
#include "GeometryObject.hpp"

namespace Helios {

	class SurfaceObject;

	class Surface {

	public:

		/* Name of this object */
		static std::string name() {return "surface";}

		/* Information about the surfaces */
		enum SurfaceInfo {
			NONE       = 0,
			REFLECTING = 1,
			VACUUM = 2
		};

		/* Surface constructor function */
		typedef Surface(*(*Constructor)(const SurfaceObject*));
		/* Friendly factory */
		friend class SurfaceFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Surface& q);

		/* Exception */
		class BadSurfaceCreation : public std::exception {
			std::string reason;
		public:
			BadSurfaceCreation(const SurfaceId& surid, const std::string& msg) {
				reason = "Cannot create surface " + toString(surid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadSurfaceCreation() throw() {/* */};
		};

		/* Sense of the coordinate respect this surface */
		inline bool sense(const Coordinate& pos) const {return (function(pos) >= 0);}
		/* Return a normal vector at some point on the surface, needed for reflection */
		virtual void normal(const Coordinate& point, Direction& vnormal) const = 0;
		/* Determine distance to intersection with the surface. Returns whether it hits and pass back what the distance is. */
		virtual bool intersect(const Coordinate& pos, const Direction& dir, const bool& sense, double& distance) const  = 0;
		/* Get the name of this surface */
		virtual std::string getName() const = 0;

		/* Comparison operator */
		bool operator==(const Surface& sur) {
			if (typeid(*this) != typeid(sur))
				return false;
			return compare(sur);
		}

		/* Add a neighbor cell of this surface */
		void addNeighborCell(const bool& sense, Cell* cell);
		/* Get neighbor cells of this surface */
		const std::vector<Cell*>& getNeighborCell(const bool& sense) const;

		/* Return the user ID associated with this surface. */
		const SurfaceId& getUserId() const {return surfid;}
		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalSurfaceId& internal) {int_surfid = internal;}
		/* Return the internal ID associated with this surface. */
		const InternalSurfaceId& getInternalId() const {return int_surfid;}
		/* Get the extra information of the surface */
		SurfaceInfo getFlags() const {return flag;}
		/* Set different options for the surfaces */
		void setFlags(SurfaceInfo new_flag) {flag = new_flag;}

		/* Mathematically define a surface as a collection of points that satisfy this equation */
		virtual double function(const Coordinate& pos) const = 0;

		/* Cross a surface, i.e. find next cell. Of course, this should be called on a position located on the surface */
		void cross(const Coordinate& position, const bool& sense, const Cell*& cell) const ;

		/*
		 * Return a new instance of the surface translated (same flags and userId)
		 * The return *type* is not necessarily the same of the original class.
		 */
		virtual Surface* transformate(const Direction& trans) const = 0;

		virtual ~Surface() {/* */};

	protected:
		/* Default, used only on factory */
		Surface() : surfid(), flag(NONE), int_surfid(0) {/* */};
		/* Constructor from id and flags */
		Surface(const SurfaceId& surfid, const SurfaceInfo& flag) : surfid(surfid), flag(flag), int_surfid(0) {/* */};
		/* Create surface from user id */
		Surface(const SurfaceObject* definition);
		/* Prevent copy */
		Surface(const Surface& surface);
		Surface& operator= (const Surface& other);

		/* Print internal parameters of the surface */
		virtual void print(std::ostream& out) const = 0;
		/* Virtual comparison operator, to avoid duplicated surfaces on the geometry */
		virtual bool compare(const Surface& sur) const = 0;

		/* Get the constructor function (used by the parser) */
		virtual Constructor constructor() const = 0;

	private:
		/* User's identification of this surface */
		SurfaceId surfid;
		/* Information about the surface */
		SurfaceInfo flag;
		/* Internal identification of this surface */
		InternalSurfaceId int_surfid;
		/* Neighbor cells */
		std::vector<Cell*> neighbor_pos;
		std::vector<Cell*> neighbor_neg;
	};

	inline void Surface::addNeighborCell(const bool& sense, Cell* cell) {
		if(sense)
			neighbor_pos.push_back(cell);
		else
			neighbor_neg.push_back(cell);
	}

	inline const std::vector<Cell*>& Surface::getNeighborCell(const bool& sense) const {
		if(sense)
			return neighbor_pos;
		else
			return neighbor_neg;
	}

	/* Cross a surface, i.e. find next cell. Of course, this should be called on a position located on the surface */
	inline void Surface::cross(const Coordinate& position, const bool& sense, const Cell*& cell) const {
		/* Set to zero */
		cell = 0;
		const std::vector<Cell*>& neighbor = getNeighborCell(not sense);
		std::vector<Cell*>::const_iterator it_neighbor = neighbor.begin();
		for( ; it_neighbor != neighbor.end() ; ++it_neighbor) {
			cell = (*it_neighbor)->findCell(position,this);
			if(cell) break;
		}
	}

	class SurfaceObject : public GeometryObject {
		SurfaceId userSurfaceId;
		std::string type;
		std::vector<double> coeffs;
		Surface::SurfaceInfo flags;
	public:
		SurfaceObject() : GeometryObject(Surface::name()) {/* */}
		SurfaceObject(const SurfaceId& userSurfaceId, const std::string& type,
				   const std::vector<double>& coeffs, const Surface::SurfaceInfo& flags = Surface::NONE) :
				   GeometryObject(Surface::name()), userSurfaceId(userSurfaceId), type(type),
				   coeffs(coeffs), flags(flags) {/* */}
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
		~SurfaceObject() {/* */}
	};

	class SurfaceFactory {

		/* Map of surfaces types and constructors */
		std::map<std::string, Surface::Constructor> constructor_table;

	public:
		/* Prevent construction or copy */
		SurfaceFactory();

		/* Register a new surface */
		void registerSurface(const Surface& surface);

		/* Create a new surface */
		Surface* createSurface(const SurfaceObject* definition) const;

		virtual ~SurfaceFactory() {/* */}
	};

	/* Output surface information */
	std::ostream& operator<<(std::ostream& out, const Surface& q);

} /* namespace Helios */

#endif /* SURFACE_HPP_ */
