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

#ifndef ENERGYLAW1_HPP_
#define ENERGYLAW1_HPP_

#include "AceEnergyLaw.hpp"
#include "EnergyTabular.hpp"

namespace Helios {
namespace AceReaction {

/* ---------- Tabular EquiProbable Energy Bins (law 1) ---------- */

	/* Sample outgoing energy using EquiProbable energy bins */
	struct EnergyEquiBins  {
		/* Outgoing energy values */
		std::vector<double> out;

		EnergyEquiBins(const std::vector<double>& out) : out(out) {/* */}

		void operator()(Random& random, double& energy, double& mu) const {
			/* Get number of equi-probable bins */
			size_t nbins = out.size() - 1;
			/* Sample random number */
			double chi = random.uniform();
			/* Sample bin */
			size_t pos = (size_t) (chi * nbins);
			/* Get interpolated energy */
			energy = out[pos] + (nbins * chi - pos) * (out[pos + 1] - out[pos]);
		}

		void print(std::ostream& sout) const {
			sout << " * Equiprobable Energy bins " << endl;
			for(size_t i = 0 ; i < out.size() ; ++i)
				sout << (int)i << scientific << setw(15) << out[i] << endl;
		}

		~EnergyEquiBins() {/* */}
	};

	class EnergyLaw1 : public EnergyOutgoingTabular<EnergyEquiBins> {
		typedef Ace::EnergyDistribution::Law1 Law1;
	public:
		EnergyLaw1(const Law* ace_data);
		~EnergyLaw1() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */
#endif /* ENERGYLAW1_HPP_ */
