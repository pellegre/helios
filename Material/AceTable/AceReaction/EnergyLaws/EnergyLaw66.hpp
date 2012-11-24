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

#ifndef ENERGYLAW66_HPP_
#define ENERGYLAW66_HPP_

#include "../../../../Common/EndfInterpolate.hpp"
#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- N-body phase space distribution  ---------- */

	class EnergyLaw66: public Helios::AceReaction::AceEnergyLaw {
		typedef Ace::EnergyDistribution::Law66 Law66;
		/* Cast to law 7 */
		const Law66* cast(const Law* law) const {return static_cast<const Law66*>(law);}
		/* Number of bodies in the phase space */
		int npxs;
		/* Total mass ratio for the NPSX particles */
		double ap;
		/* Q-value of the reaction */
		double q;
		/* Atomic weight ratio of the original target nucleus */
		double awr;
	public:
        EnergyLaw66(const Law* ace_data, double q, double awr) : AceEnergyLaw(ace_data),
        	npxs(cast(ace_data)->npxs), ap(cast(ace_data)->ap), q(q), awr(awr) {/* */}

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {
			/* Incident energy */
			double ein(particle.getEnergy().second);
			/* Calculate maximum energy */
			energy = ((ap - 1.0)/ap)*((awr/(awr + 1.0))*ein + q);
			/* Sample random numbers */
			double rnd1(0.0), rnd2(0.0), rnd3(0.0), rnd4(0.0);

			/* First round */
			do {
			    rnd1 = random.uniform();
			    rnd1 = rnd1*rnd1;
			    rnd2 = random.uniform();
			    rnd2 = rnd1 + rnd2*rnd2;
			} while (rnd2 > 1.0);

			/* Second round */
			do {
			    rnd3 = random.uniform();
			    rnd3 = rnd3*rnd3;
			    rnd4 = random.uniform();
			    rnd4 = rnd3 + rnd4*rnd4;
			} while (rnd4 > 1.0);

			/* Auxiliary variable */
			double arg(1.0);

			/* Get value of p */
			if (npxs == 3) arg = random.uniform();
			else if (npxs == 4) arg = random.uniform()*random.uniform();
			else if (npxs == 5) arg = random.uniform()*random.uniform()*random.uniform()*random.uniform();

			/* Get x and y */
			double x = -rnd1*log(rnd2)/rnd2 - log(random.uniform());
			double y = -rnd3*log(rnd4)/rnd4 - log(arg);

			/* Final energy */
			energy = (x/(x + y))*energy;

			/* Scattering cosine is sampled isotropically */
			mu = 1.0 - 2.0*random.uniform();
		}

		/* Print internal information of the law */
		void print(std::ostream& out) const {
			out << " npxs = " << npxs << endl;
			out << " ap = " << ap << endl;
		}

		~EnergyLaw66() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */


#endif /* ENERGYLAW66_HPP_ */
