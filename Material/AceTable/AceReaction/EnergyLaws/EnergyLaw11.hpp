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

#ifndef ENERGYLAW11_HPP_
#define ENERGYLAW11_HPP_

#include "../../../../Common/EndfInterpolate.hpp"
#include "AceEnergyLaw.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Energy Dependent Watt Spectrum  ---------- */

	class EnergyLaw11: public Helios::AceReaction::AceEnergyLaw {
		typedef Ace::EnergyDistribution::Law11 Law11;
		/* Cast to law 7 */
		const Law11* cast(const Law* law) const {return static_cast<const Law11*>(law);}
        /* ENDF interpolate lawa */
        EndfInterpolate endf_interpolate_a;
        EndfInterpolate endf_interpolate_b;
		/* Incident energy */
        std::vector<double> eina;
        std::vector<double> einb;
        /* Coefficients */
        std::vector<double> a;
        std::vector<double> b;
        /* Restriction energy */
        double u;
	public:
        EnergyLaw11(const Law* ace_data) : AceEnergyLaw(ace_data),
        	endf_interpolate_a(cast(ace_data)->inta.nbt, cast(ace_data)->inta.aint),
        	endf_interpolate_b(cast(ace_data)->intb.nbt, cast(ace_data)->intb.aint),
        	eina(cast(ace_data)->eina), einb(cast(ace_data)->einb),
			a(cast(ace_data)->a), b(cast(ace_data)->b), u(cast(ace_data)->u) {
			/* Sanity check */
			assert(eina.size() == a.size());
			assert(einb.size() == b.size());
        }

		/* Sample scattering outgoing energy */
		void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const {
			/* Incident energy */
			double ienergy(particle.getEnergy().second);
			/* Get coefficients */
			double acoeff = endf_interpolate_a.interpolate(eina.begin(), eina.end(), a.begin(), a.end(), ienergy);
			double bcoeff = endf_interpolate_b.interpolate(einb.begin(), einb.end(), b.begin(), b.end(), ienergy);

		    /* Calculate constants */
		    double c = 1.0 + acoeff*bcoeff/8.0;
		    double g = sqrt(c*c - 1.0) + c;

		    /* Sample energy (p. 2-45 in MCNP4C manual) */
		    do {
		    	/* Auxiliary variable */
			    double d;
			    do {
					double rnd1 = log(random.uniform());
					d = (1.0 - g)*(1.0 - rnd1) - log(random.uniform());
					energy = -acoeff*g*rnd1;
			    } while (d*d > bcoeff*energy);
		    } while (energy > ienergy - u);
		}

		/* Print internal information of the law */
		void print(std::ostream& out) const;

		~EnergyLaw11() {/* */}
	};
} /* namespace AceReaction */
} /* namespace Helios */
#endif /* ENERGYLAW11_HPP_ */
