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

#ifndef AXISNAME_HPP_
#define AXISNAME_HPP_

namespace Helios {

	/* Axis */
	const int xaxis = 0;
	const int yaxis = 1;
	const int zaxis = 2;

	/* Get the name of an axis */
	template<int axis>
	static std::string getAxisName() {
		switch(axis) {
		case xaxis :
			return "x";
			break;
		case yaxis :
			return "y";
			break;
		case zaxis :
			return "z";
			break;
		}
		return "";
	}

	/* Get the name of an axis (no - template) */
	static std::string getAxisName(int axis) {
		switch(axis) {
		case xaxis :
			return "x";
			break;
		case yaxis :
			return "y";
			break;
		case zaxis :
			return "z";
			break;
		}
		return "";
	}

	/* Return the plane perpendicular to an axis */
	template<int axis>
	static std::string getPlaneName() {
		switch(axis) {
		case xaxis :
			return "yz";
			break;
		case yaxis :
			return "xz";
			break;
		case zaxis :
			return "xy";
			break;
		}
		return "--";
	}

	template<int axis>
	static double getAbscissa(const Coordinate& value) {
		switch(axis) {
		case xaxis :
			return value[yaxis];
			break;
		case yaxis :
			return value[zaxis];
			break;
		case zaxis :
			return value[xaxis];
			break;
		}
		return 0;
	}

	template<int axis>
	static double getOrdinate(const Coordinate& value) {
		switch(axis) {
		case xaxis :
			return value[zaxis];
			break;
		case yaxis :
			return value[xaxis];
			break;
		case zaxis :
			return value[yaxis];
			break;
		}
		return 0;
	}

}

#endif /* AXISNAME_HPP_ */
