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

#ifndef FISSIONPOLICY_HPP_
#define FISSIONPOLICY_HPP_

#include "../AceReader/ReactionContainer.hpp"
#include "../AceReader/NeutronTable.hpp"
#include "../AceReaction/NuSampler.hpp"
#include "../../../Environment/McModule.hpp"
#include "../../../Common/Common.hpp"
#include "../../Grid/MasterGrid.hpp"
#include "../../Isotope.hpp"

namespace Helios {

	class AceIsotopeBase;

	/* Base class to deal with fission treatment */
	class FissilePolicyBase {
		/* Constant reference to a CHILD grid */
		const ChildGrid* iso_child_grid;
	protected:
		/* Fission cross section */
		Ace::CrossSection fission_xs;
	public:
		FissilePolicyBase(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Get fission cross section */
		double getFissionXs(Energy& energy) const;

		~FissilePolicyBase() {}
	};

	class NonFissile : public FissilePolicyBase {

	public:
		/* Constructor from table */
		NonFissile(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
			FissilePolicyBase(_isotope, _table, _child_grid) {/* */};

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			return 0;
		};

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return 0.0;
		}

		/* Check if the isotope is fissile */
		bool isFissile() const {return false;}

		~NonFissile() {/* */}
	};

	class TotalNuFission : public FissilePolicyBase {
		/*
		 * Fission reaction. As always, this reaction is treated separately.
		 */
		Reaction* fission_reaction;
		/* NU sampler */
		AceReaction::NuSampler* total_nu;
		/*
		 * Flag if the isotope has fission information
		 */
		bool fissile;

	public:
		/* Constructor from table */
		TotalNuFission(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			return fission_reaction;
		};

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return total_nu->getNuBar(energy.second);
		}

		/* Check if the isotope is fissile */
		bool isFissile() const {return fissile;}

		~TotalNuFission() {/* */}
	};

	class TotalNuChanceFission : public FissilePolicyBase {
		/*
		 * Fission reaction. As always, this reaction is treated separately.
		 */
		Reaction* fission_reaction;
		/* NU sampler */
		AceReaction::NuSampler* total_nu;
	public:
		/* Constructor from table */
		TotalNuChanceFission(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			return fission_reaction;
		};

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return total_nu->getNuBar(energy.second);
		}

		/* Check if the isotope is fissile */
		bool isFissile() const {return true;}

		~TotalNuChanceFission() {/* */}
	};

}

#endif /* FISSIONPOLICY_HPP_ */
