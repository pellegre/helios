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

#include "../AceReader/ReactionContainer.hpp"
#include "../../../Common/Common.hpp"
#include "../../../Common/XsSampler.hpp"
#include "../../Grid/MasterGrid.hpp"
#include "InelasticScattering.hpp"
#include "NuSampler.hpp"

namespace Helios {

namespace AceReaction {

	class Fission: public GenericReaction {

	public:
		Fission(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction);

		/* Print internal information of the reaction */
		void print(std::ostream& out) const;

		/* Sample a fission neutron (the weight of the neutron is NOT modified) */
		void operator()(Particle& particle, Random& random) const {
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

	class ChanceFission : public Reaction {
		/* Fission cross section (reference) */
		const Ace::CrossSection& fission_xs;
		/* Constant reference to a CHILD grid */
		const ChildGrid* child_grid;
		/* Fission chance (1st, 2nd, 3rd and 4th) reaction sampler (using an interpolation factor) */
		XsSampler<Reaction*>* chance_sampler;
	public:
		ChanceFission(std::vector<pair<Reaction*,const Ace::CrossSection*> >& reaction_array,
					const Ace::CrossSection& fission_xs, const ChildGrid* child_grid) :
					Reaction(18), fission_xs(fission_xs), child_grid(child_grid) {
			/* Sanity check */
			assert(reaction_array.size() == 4);
			/* Create sampler */
			chance_sampler = new XsSampler<Reaction*>(reaction_array);
		}

		/* Print internal information of the reaction */
		void print(std::ostream& out) const;

		/* Sample a fission neutron (the weight of the neutron is NOT modified) */
		void operator()(Particle& particle, Random& random) const {
			/* Sample the fission reaction at this energy */
			double factor;
			size_t idx = child_grid->index(particle.erg(), factor);
			double xs = factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
			Reaction* chance_reaction = chance_sampler->sample(idx, xs * random.uniform(), factor);
			/* Apply reaction */
			(*chance_reaction)(particle, random);
		}

		~ChanceFission() {
			/* Delete sampler */
			delete chance_sampler;
		}
	};
}

} /* namespace Helios */
#endif /* FISSIONREACTION_HPP_ */
