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

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <vector>
#include <sstream>
#include <string>
#include <blitz/array.h>
#include <trng/lcg64.hpp>
#include <trng/uniform01_dist.hpp>

#include "../Log/Log.hpp"
#include "FloatingGtest.hpp"

/* Some types used in the program */

namespace Helios {
	BZ_USING_NAMESPACE(blitz)

	/* Define a coordinate */
	typedef TinyVector<double,3> Coordinate;
	/* Define a direction */
	typedef TinyVector<double,3> Direction;

	/* Matrices and arrays (floating point) */
	typedef Array<double,1> Vector;
	typedef Array<double,2> Matrix;

	/* Matrices and arrays (integers) */
	typedef Array<int,1> IntVector;
	typedef Array<int,2> IntMatrix;
	typedef Array<int,3> IntCube;

	/* Epsilon of floating point */
	const double eps = std::numeric_limits<double>::epsilon();

	/* Surface ID defined by the user */
	typedef unsigned int SurfaceId;
	/* Cell ID defined by the user */
	typedef unsigned int CellId;
	/* Universe ID defined by the user */
	typedef unsigned int UniverseId;
	/* Material ID defined by the user, usually a string */
	typedef std::string MaterialId;

	/* Surface ID used internally */
	typedef unsigned int InternalSurfaceId;
	/* Cell ID used internally */
	typedef unsigned int InternalCellId;
	/* Universe ID used internally */
	typedef unsigned int InternalUniverseId;
	/* Material ID used internally */
	typedef unsigned int InternalMaterialId;

	/* Energy stuff */
	typedef double Energy;                             /* Energy of the particle in eVs */
	typedef unsigned int EnergyIndex;                  /* Index used to access energies grids */
	typedef std::pair<EnergyIndex,Energy> EnergyPair;  /* Pair of energy value and the closest (lower) index on the grid */

	/* Get a value from a string */
	template<typename T>
	static inline T fromString(const std::string& str) {std::istringstream s(str);T t;s >> t;return t;}

	/* Convert to string */
	template<typename T>
	static inline std::string toString(const T& t) {std::ostringstream s;s << t;return s.str();}

	/* Axis */
	const int xaxis = 0;
	const int yaxis = 1;
	const int zaxis = 2;

	/* ---- Comparison function, ONLY FOR "ADMINISTRATIVE" CODE */

	/* Compare two floating point numbers */
	template<class T>
	static inline bool compareFloating(const T& left, const T& right) {
		/* From google testing framework */
		const FloatingPoint<T> lhs(left), rhs(right);
		return lhs.AlmostEquals(rhs);
	}

	/* Compare tiny vectors */
	static inline bool compareTinyVector(const Coordinate& a, const Coordinate& b) {
		return (compareFloating(a[0],b[0]) && compareFloating(a[1],b[1]) && compareFloating(a[2],b[2]));
	}

}

#endif /* TYPES_H_ */
