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

#ifndef ACEISOTOPEBASE_HPP_
#define ACEISOTOPEBASE_HPP_

#include "AceReader/ReactionContainer.hpp"
#include "AceReader/NeutronTable.hpp"
#include "AceReaction/NuSampler.hpp"
#include "../../Environment/McModule.hpp"
#include "../../Common/Common.hpp"
#include "../Grid/MasterGrid.hpp"
#include "../Isotope.hpp"

namespace Helios {

template<typename TypeReaction> class XsSampler;

	/* Isotope related to an ACE table. */
	class AceIsotopeBase : public Isotope {
		/* Friendly factory */
		friend class AceIsotopeFactory;

	protected:

		/* Auxiliary function to get the probability of a reaction */
		double getProb(Energy& energy, const Ace::CrossSection& xs) const;

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

		/* Absorption cross section */
		Ace::CrossSection absorption_xs;
		/* Elastic cross section */
		Ace::CrossSection elastic_xs;
		/* Inelastic cross section */
		Ace::CrossSection inelastic_xs;

		/* -- Reactions */

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

		AceIsotopeBase(const Ace::NeutronTable& _table, const ChildGrid* child_grid);

		/* Get isotope information */
		double getAwr() const {return aweight;}

		/* Get temperature (in MeVs) */
		double getTemperature() const {return temperature;}

		/* Get absorption probability */
		double getAbsorptionProb(Energy& energy) const;
		/* Get elastic probability */
		double getElasticProb(Energy& energy) const;

		/* Get total cross section */
		double getTotalXs(Energy& energy) const;

		/* Fission treatment of the isotope */

		/* Get fission probability */
		double getFissionProb(Energy& energy) const;

		/* Get fission cross section */
		virtual double getFissionXs(Energy& energy) const = 0;
		/* Fission reaction */
		virtual Reaction* fission(Energy& energy, Random& random) const = 0;
		/* Get average NU-bar at some energy */
		virtual double getNuBar(const Energy& energy) const = 0;

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

		~AceIsotopeBase();
	};

	/*
	 * Ace factory, this should create an ACE isotope with information obtained
	 * from an ACE table.
	 */
	class AceIsotopeFactory {
		/* Master grid */
		MasterGrid* master_grid;
	public:
		AceIsotopeFactory(MasterGrid* master_grid) : master_grid(master_grid) {/* */}

		/* Returns an ACE isotope */
		AceIsotopeBase* createIsotope(const Ace::NeutronTable& table) const;

		~AceIsotopeFactory() {/* */}
	};

} /* namespace Helios */

#endif /* ACEISOTOPEBASE_HPP_ */
