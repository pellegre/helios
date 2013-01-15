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

#ifndef ACEISOTOPE_HPP_
#define ACEISOTOPE_HPP_

#include "../AceIsotopeBase.hpp"

namespace Helios {

template<class FissionPolicy>
class AceIsotope : public AceIsotopeBase, public FissionPolicy {
	/* The policy has the fission cross section */
	using FissionPolicy::fission_xs;
public:
	AceIsotope(const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
		AceIsotopeBase(_table, _child_grid), FissionPolicy(this, _table, _child_grid) {/* */};

	/* Get fission cross section */
	double getFissionXs(Energy& energy) const {
		return FissionPolicy::getFissionXs(energy);
	}

	/* Fission reaction */
	Reaction* fission(Energy& energy, Random& random) const {
		return FissionPolicy::fission(energy, random);
	}
	/* Get average NU-bar at some energy */
	double getNuBar(const Energy& energy) const {
		return FissionPolicy::getNuBar(energy);
	}

	virtual ~AceIsotope() {};
};

} /* namespace Helios */
#endif /* ACEISOTOPE_HPP_ */
