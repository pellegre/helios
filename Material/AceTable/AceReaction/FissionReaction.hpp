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

#ifndef FISSIONREACTION_HPP_
#define FISSIONREACTION_HPP_

#include "InelasticScattering.hpp"
#include "NuSampler.hpp"

namespace Helios {

namespace AceReaction {

	class Fission: public GenericReaction {
		/* NU sampler */
		NuSampler* prompt_nu;
	public:
		Fission(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction);
		/* Print internal information of the reaction */
		void print(std::ostream& out) const;

		/* Sample a fission neutron (the weight of the neutron is multiplied by NU) */
		void operator()(Particle& particle, Random& random) const {
			/* Sample NU  */
			double nubar = prompt_nu->getNu(particle.erg().second);
			/* Integer part */
			int nu = (int) nubar;
			if (random.uniform() < nubar - (double)nu)
				nu++;

			/* Just multiply the particle weight */
			particle.wgt() *= (double)nu;

			/* Sample new scattering cosine */
			double mu;
			sampleCosine(particle, random, mu);
			/* Sample new energy */
			double energy;
			sampleEnergy(particle, random, energy, mu);

			/* Set new direction */
			azimutalRotation(mu, particle.dir(), random);

			/* Set new energy */
			particle.erg().second = energy;
		}

		~Fission();
	};

}

} /* namespace Helios */
#endif /* FISSIONREACTION_HPP_ */
