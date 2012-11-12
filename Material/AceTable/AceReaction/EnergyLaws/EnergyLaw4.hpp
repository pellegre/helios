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

#ifndef ENERGYLAW4_HPP_
#define ENERGYLAW4_HPP_

#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Continuous tabular distribution (law 4) ---------- */

	/* Sample outgoing energy using a tabular distribution */
	class EnergyTabular : public TabularDistribution /* defined on AceReactionCommon.hpp */ {
	public:

		EnergyTabular(const Ace::EnergyDistribution::Law4::EnergyData& ace_energy) :
			TabularDistribution(ace_energy.intt, ace_energy.eout, ace_energy.pdf, ace_energy.cdf)
		{/* */}

		double operator()(Random& random) const {
			return TabularDistribution::operator()(random);
		}

		void print(std::ostream& out) const {
			out << " * Energy Tabular Distribution " << endl;
			TabularDistribution::print(out);
		}

		~EnergyTabular() {/* */}
	};

	class EnergyLaw4 : public AceEnergyLaw, public TableSampler<EnergyTabular*> {
		typedef Ace::EnergyDistribution::Law4 Law4;
	public:
		EnergyLaw4(const Law* ace_data);

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {
			/* Get particle energy */
			double initial_energy = particle.getEnergy().second;
			/* Interpolation data */
			std::pair<size_t,double> inter;
			/* Sample cosine table */
			EnergyTabular* energy_table = TableSampler<EnergyTabular*>::sample(initial_energy, random, inter);
			/* Once we got the table, sample the new energy */
			energy = (*energy_table)(random);

			/* -- Scaled interpolation */

			size_t idx = inter.first;
			double factor = inter.second;

			/* Minimum energy */
			double emin = tables[idx]->out[0] + factor * (tables[idx + 1]->out[0] - tables[idx]->out[0]);
			/* Maximum energy */
			size_t last1 = tables[idx]->out.size() - 1;
			size_t last2 = tables[idx + 1]->out.size() - 1;
			double emax = tables[idx]->out[last1] + factor * (tables[idx + 1]->out[last2] - tables[idx]->out[last1]);

			/* Get bounds on the sampled table */
			size_t last = energy_table->out.size() - 1;
			double eo = energy_table->out[0];
			double ek = energy_table->out[last];

			/* Finally, scale the sampled energy */
			energy = emin + ((energy - eo) * (emax - emin))/(ek - eo);
		}

		void print(std::ostream& out) const;

		~EnergyLaw4();
	};

} /* namespace AceReaction */
} /* namespace Helios */
#endif /* ENERGYLAW4_HPP_ */
