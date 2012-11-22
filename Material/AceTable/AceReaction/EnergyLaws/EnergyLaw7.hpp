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

#ifndef ENERGYLAW7_HPP_
#define ENERGYLAW7_HPP_

#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Level Scattering  ---------- */

	class EnergyLaw7: public Helios::AceReaction::AceEnergyLaw {
		typedef Ace::EnergyDistribution::Law7 Law7;

	public:
		EnergyLaw7(const Law* ace_data) : AceEnergyLaw(ace_data) {
			const Law7* law_data = dynamic_cast<const Law7*>(ace_data);
			Ace::EnergyDistribution::InterScheme int_sch = law_data->int_sch;
			cout << " = " << endl;
			for(int i = 0 ; i < int_sch.nr ; ++i) {
				cout << int_sch.nbt[i] << " " << int_sch.aint[i] << endl;
			}
			cout << " = " << endl;

		}

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {

		}

		void print(std::ostream& out) const {

		}

		~EnergyLaw7() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */

#endif /* ENERGYLAW7_HPP_ */
