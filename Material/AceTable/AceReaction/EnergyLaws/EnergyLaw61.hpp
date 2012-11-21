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

#ifndef ENERGYLAW61_HPP_
#define ENERGYLAW61_HPP_

#include "AceEnergyLaw.hpp"
#include "EnergyTabular.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Like law 44 but with an angular distribution instead of Kalbach-87 Formalism (law 61) ---------- */

	/* Sample outgoing energy using a tabular distribution */
	class AngularTabular : public TabularDistribution /* defined on AceReactionCommon.hpp */ {

	public:
		AngularTabular(const Ace::EnergyDistribution::Law61::EnergyData& ace_energy) :
			TabularDistribution(ace_energy.intt, ace_energy.eout, ace_energy.pdf, ace_energy.cdf)
		{/* */}

		void operator()(Random& random, double& energy, double& mu) const {
			/* Index on the outgoing grid */
			size_t idx;
			/* Set energy */
			energy = TabularDistribution::operator()(random, idx);

		}

		void print(std::ostream& out) const {
			out << " * Energy Tabular Distribution " << endl;
			TabularDistribution::print(out);
		}

		~AngularTabular() {/* */}
	};

	class EnergyLaw61 : public EnergyOutgoingTabular<AngularTabular> {
		typedef Ace::EnergyDistribution::Law61 Law61;
	public:
		EnergyLaw61(const Law* ace_data);
		~EnergyLaw61() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */

#endif /* ENERGYLAW61_HPP_ */
