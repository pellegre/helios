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

#ifndef TRANSFORMATION_HPP_
#define TRANSFORMATION_HPP_

#include "Surface.hpp"

namespace Helios {

class Transformation {

	/* A translation transformation */
	Direction translation;
	/* A rotation (degrees around each of the 3 axis) */
	Direction rotation;

public:
	Transformation(const Direction& translation = Direction(0,0,0), const Direction& rotation = Direction(0,0,0))
					: translation(translation), rotation(rotation) {/* */}

	/* Returns a new instance of a cloned transformed surface */
	Surface* operator()(const Surface* surface) const { return surface->transformate(translation); }

	/* Sum transformations */
	const Transformation operator+(const Transformation& right) const {
		return Transformation(right.translation + translation, right.rotation + rotation);
	}

	~Transformation() {/* */}
};

} /* namespace Helios */
#endif /* TRASNFORMATION_HPP_ */
