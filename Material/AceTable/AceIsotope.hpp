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

#ifndef ACEISOTOPE_HPP_
#define ACEISOTOPE_HPP_

#include "AceReader/ReactionContainer.hpp"
#include "AceReaction/NuSampler.hpp"
#include "../../Environment/McModule.hpp"
#include "../../Common/Common.hpp"
#include "../Grid/MasterGrid.hpp"
#include "../Isotope.hpp"

namespace Helios {

template<typename TypeReaction> class XsSampler;

	/* Isotope related to an ACE table. */
	class AceIsotope : public Isotope {

		/* Auxiliary function to get the probability of a reaction */
		double getProb(Energy& energy, const Ace::CrossSection& xs) const;

		/* Auxiliary method to set the fission reaction stuff */
		void setFissionReaction();

		/* -- General data */

		/* Reference to a neutron container (obtained from the AceReader) */
		Ace::ReactionContainer reactions;

		/* Atomic weight ratio */
		double aweight;
		/* Temperature at which the data were processed (in MeV) */
		double temperature;

		/* Constant reference to a CHILD grid */
		const ChildGrid* child_grid;

		/* Print isotope information */
		void print(std::ostream& out) const;

		/* -- Cross sections */

		/* Total cross section */
		Ace::CrossSection total_xs;
		/* Fission cross section */
		Ace::CrossSection fission_xs;
		/* Absorption cross section */
		Ace::CrossSection absorption_xs;
		/* Elastic cross section */
		Ace::CrossSection elastic_xs;
		/* Inelastic cross section */
		Ace::CrossSection inelastic_xs;

		/* -- Reactions */

		/*
		 * Fission reaction. As always, this reaction is treated separately. Pointer
		 * is NULL for non-fissiles isotopes.
		 */
		Reaction* fission_reaction;
		/* NU sampler */
		AceReaction::NuSampler* total_nu;

		/* Elastic reaction of this isotope. This reaction always exist */
		Reaction* elastic_scattering;

		/* Map of MT numbers and reactions */
		std::map<int,Reaction*> reaction_map;

		/* Secondary particle reaction sampler (using an interpolation factor) */
		XsSampler<Reaction*>* secondary_sampler;

	public:

		/* Threshold values */
		static double energy_freegas_threshold;
		static double awr_freegas_threshold;

		AceIsotope(const Ace::ReactionContainer& reactions, const ChildGrid* child_grid);

		/* Get isotope information */
		double getAwr() const {return aweight;}

		/* Get temperature (in MeVs) */
		double getTemperature() const {return temperature;}

		/* Get absorption probability */
		double getAbsorptionProb(Energy& energy) const;
		/* Get fission probability */
		double getFissionProb(Energy& energy) const;
		/* Get elastic probability */
		double getElasticProb(Energy& energy) const;

		/* Get total cross section */
		double getTotalXs(Energy& energy) const;

		/* Get fission cross section */
		double getFissionXs(Energy& energy) const;

		/* Fission reaction */
		Reaction* fission() const {
			return fission_reaction;
		};

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return total_nu->getNuBar(energy.second);
		}

		/* Elastic reaction */
		Reaction* elastic() const {
			return elastic_scattering;
		}

		/* Inelastic Scattering (we should sample the reaction) */
		Reaction* inelastic(Energy& energy, Random& random) const;

		/*
		 * Get reaction from an MT number (thrown an exception if the reaction number does not exist)
		 * Each created reaction is managed by the isotope.
		 */
		Reaction* getReaction(InternalId mt);

		~AceIsotope();
	};

	/*
	 * Ace factory, this should create an ACE isotope with information obtained
	 * from an ACE table.
	 */
	class AceIsotopeFactory {
	public:
		AceIsotopeFactory() {/* */}
		~AceIsotopeFactory() {/* */}
	};

} /* namespace Helios */

#endif /* ACEISOTOPE_HPP_ */
