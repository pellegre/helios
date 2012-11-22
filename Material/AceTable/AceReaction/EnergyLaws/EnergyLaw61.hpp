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

#include "../MuSampler.hpp"
#include "AceEnergyLaw.hpp"
#include "EnergyTabular.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Like law 44 but with an angular distribution instead of Kalbach-87 Formalism (law 61) ---------- */

	/* Sample outgoing energy using a tabular distribution */
	class AngularTabular : public TabularDistribution /* defined on AceReactionCommon.hpp */ {
		/* Angular array */
		typedef Ace::AngularDistribution::AngularArray AceAngular;
		/* Cosine table associated to this energy outgoing bin */
		std::vector<CosineTable*> cosine_table;
	public:
		AngularTabular(const Ace::EnergyDistribution::Law61::EnergyData& ace_energy) :
			TabularDistribution(ace_energy.intt, ace_energy.eout, ace_energy.pdf, ace_energy.cdf)
		{
			/* Create the cosine table */
			for(std::vector<AceAngular*>::const_iterator it = ace_energy.adist.begin() ; it != ace_energy.adist.end() ; ++it)
				cosine_table.push_back(MuTable::tableBuilder((*it)));
			/* Sanity check */
			assert(cosine_table.size() == out.size());
		}

		void operator()(Random& random, double& energy, double& mu) const {
			/* Index on the outgoing grid */
			size_t idx;
			/* Get random number (we need it to sample the angular bin) */
			double chi = random.uniform();
			/* Set energy */
			energy = TabularDistribution::operator()(chi, idx);
			/* Sample the scattering cosine */
			CosineTable* cosine(0);
			if(iflag == 1) {
				/* Histogram interpolation */
				cosine = cosine_table[idx];
			} else {
				/* Linear-Linear interpolation */
			    if (chi - cdf[idx] < cdf[idx + 1] - chi)
					cosine = cosine_table[idx];
				else
					cosine = cosine_table[idx + 1];
			}
			/* Once we got the table, sample the scattering cosine */
			mu = (*cosine)(random);
		}

		void print(std::ostream& sout) const {
			sout << " * Energy Tabular Distribution " << endl;
			TabularDistribution::print(sout);
			for(size_t i = 0 ; i < cosine_table.size() ; ++i) {
				sout << "energy = " << scientific << out[i] << endl;
				cosine_table[i]->print(sout);
			}
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
