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

#ifndef ENERGYLAW3_HPP_
#define ENERGYLAW3_HPP_

#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Level Scattering  ---------- */

	class EnergyLaw3: public Helios::AceReaction::AceEnergyLaw {
		typedef Ace::EnergyDistribution::Law3 Law3;
		double ldat1;
		double ldat2;
	public:
		EnergyLaw3(const Law* ace_data) : AceEnergyLaw(ace_data) {
			const Law3* law_data = dynamic_cast<const Law3*>(ace_data);
			ldat1 = law_data->ldat1;
			ldat2 = law_data->ldat2;
		}

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {
			/* Incident energy */
			double ein = particle.getEnergy().second;
			/* Set outgoing energy (this is on center-of-mass system) */
			energy = ldat2 * (ein - ldat1);
		}

		void print(std::ostream& out) const {
			AceEnergyLaw::print(out);
			out << "  ldat1 = " << ldat1 << endl;
			out << "  ldat2 = " << ldat2 << endl;
		}

		~EnergyLaw3() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */
#endif /* ENERGYLAW3_HPP_ */
