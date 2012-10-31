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

#ifndef ACEMATERIAL_HPP_
#define ACEMATERIAL_HPP_

#include "../Material.hpp"
#include "../Grid/MasterGrid.hpp"
#include "../../Common/Sampler.hpp"

namespace Helios {

	/* Isotope related to an ACE table. */
	class AceIsotope : public Isotope {

		/* Constant reference to a CHILD grid */
		const ChildGrid* child_grid;

		/* Probabilities */
		std::vector<double> absorption_prob;
		std::vector<double> fission_prob;

	public:
		AceIsotope();

		double getAbsorptionProb(Energy& energy) const {
			double factor;
			size_t idx = child_grid->index(energy,factor);
			return factor * (absorption_prob[idx + 1] - absorption_prob[idx]) + absorption_prob[idx];
		}

		double getFissionProb(Energy& energy) const {
			double factor;
			size_t idx = child_grid->index(energy,factor);
			return factor * (fission_prob[idx + 1] - fission_prob[idx]) + fission_prob[idx];
		}

		/* Fission reaction */
		void fission(Particle& particle, Random& random) const;

		/* Just one scattering reaction (from the scattering matrix) */
		void scatter(Particle& particle, Random& random) const;

		~AceIsotope();
	};

	class AceMaterial : public Helios::Material {

		/* Total cross section */
		std::vector<double> mfp;

		/* Isotope sampler */
		Sampler<AceIsotope*>* isotope_sampler;

		/* Constant reference to a MASTER grid (managed by the AceMaterialFactory) */
		const MasterGrid* master_grid;

	public:

		/* Name of this object */
		static std::string name() {return "material";}

		AceMaterial(const AceMaterialObject* definition) {/* */};

		 /* Get the total cross section (using the energy index of the particle) */
		double getMeanFreePath(Energy& energy) const {
			double factor = master_grid->interpolate(energy);
			size_t idx = energy.first;
			return factor * (mfp[idx + 1] - mfp[idx]) + mfp[idx];
		}

		/* Sample the isotope */
		const Isotope* getIsotope(Energy& energy, Random& random) const;

		/* Print material information */
		void print(std::ostream& out) const;

		~AceMaterial();
	};

	/* Definition of an ace cross section */
	class AceMaterialObject : public MaterialObject {

	public:
		AceMaterialObject() {/* */};

		~AceMaterialObject() {/* */};
	};

	/* Material Factory */
	class AceMaterialFactory : public MaterialFactory {
	public:
		/* Prevent construction or copy */
		AceMaterialFactory() {/* */};
		/* Create a new materials */
		virtual std::vector<Material*> createMaterials(const std::vector<MaterialObject*>& definitions) const;
		virtual ~AceMaterialFactory() {/* */}
	};

} /* namespace Helios */
#endif /* ACEMATERIAL_HPP_ */
