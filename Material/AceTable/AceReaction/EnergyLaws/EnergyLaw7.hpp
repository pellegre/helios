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

#include "../../../../Common/EndfInterpolate.hpp"
#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Level Scattering  ---------- */

	class EnergyLaw7: public Helios::AceReaction::AceEnergyLaw {
		typedef Ace::EnergyDistribution::Law7 Law7;
		/* Cast to law 7 */
		const Law7* cast(const Law* law) const {return static_cast<const Law7*>(law);}
        /* ENDF interpolate law */
        EndfInterpolate endf_interpolate;
		/* Incident energy */
        std::vector<double> ein;
        /* Temperature */
        std::vector<double> t;
        /* Restriction energy */
        double u;
	public:
		EnergyLaw7(const Law* ace_data) : AceEnergyLaw(ace_data),
			endf_interpolate(cast(ace_data)->int_sch.nbt, cast(ace_data)->int_sch.aint), ein(cast(ace_data)->ein),
			t(cast(ace_data)->t), u(cast(ace_data)->u) {
			/* Sanity check */
			assert(ein.size() == t.size());
		}

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {
			/* Incident energy */
			double ienergy(particle.getEnergy().second);
			/* Get temperature */
			double temp = endf_interpolate.interpolate(ein.begin(), ein.end(), t.begin(), t.end(), ienergy);
			/* Auxiliary variables */
			double rnd1(0.0), rnd2(0.0), c(0.0);
			/* Sample outgoing energy */
		    do {
				/* Sample two random numbers */
				do {
					rnd1 = random.uniform();
					rnd2 = random.uniform();

					rnd1 = rnd1*rnd1;
					rnd2 = rnd2*rnd2;

					c = rnd1 + rnd2;
				}
				while (c > 1.0);
				/* Calculate energy */
				energy = -temp*((rnd1*log(random.uniform()))/c + log(random.uniform()));
		    } while (energy > ienergy - u);
		}

		/* Print internal information of the law */
		void print(std::ostream& out) const;

		~EnergyLaw7() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */

#endif /* ENERGYLAW7_HPP_ */
