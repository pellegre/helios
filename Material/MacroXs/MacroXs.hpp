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

#ifndef MACROXS_HPP_
#define MACROXS_HPP_

#include <map>

#include "../Material.hpp"
#include "../../Common/Sampler.hpp"

namespace Helios {

	class MacroXsObject;

	/* ---- Reactions related to macroscopic cross sections */

	namespace MacroXsReaction {

		/*
		 * -- Fission reaction
		 *   - NU is sampled and the weight of the particle is multiplied by NU.
		 *   - Isotropic direction on LAB system (macro XS don't have AWR information)
		 *   - Energy sampled from the CHI spectrum.
		 */
		class Fission : public Reaction {
			/* NU value for each group */
			std::vector<double> nu;
			/* Spectrum sampler */
			Sampler<int>* spectrum;
		public:
			Fission(const std::vector<double>& nu, const std::vector<double>& chi);
			void operator() (Particle& particle, Random& r) const {
				/* Get number of particles */
				double nubar = nu[particle.erg().first];
				/* Integer part */
				int nu = (int) nubar;
				if (r.uniform() < nubar - (double)nu)
					nu++;
				particle.wgt() *= (double)nu;
				/* New direction */
				isotropicDirection(particle.dir(),r);
				/* New group */
				particle.erg().first = spectrum->sample(0,r.uniform());
			};
			~Fission();
		};

		/*
		 * -- Scattering reaction
		 *   - Isotropic direction on LAB system (macro XS don't have AWR information)
		 *   - Energy sampled from the SIGMA-S matrix.
		 */
		class Scattering : public Reaction {
			/* Spectrum sampler */
			Sampler<int>* spectrum;
		public:
			Scattering(const std::vector<double>& sigma_scat, size_t ngroups);
			void operator() (Particle& particle, Random& r) const{
				/* New direction */
				isotropicDirection(particle.dir(),r);
				/* New group */
				particle.erg().first = spectrum->sample(particle.erg().first,r.uniform());
			};
			~Scattering();
		};

	}

	/*
	 * Isotope related to macroscopic cross section. Actually, a macro-xs is just
	 * one isotope.
	 */

	class MacroXsIsotope : public Isotope {
		/* Fission reaction */
		MacroXsReaction::Fission* fission_reaction;
		/* Scattering reaction */
		MacroXsReaction::Scattering* scattering_reaction;

		/* Probabilities */
		std::vector<double> absorption_prob;
		std::vector<double> fission_prob;
	public:
		MacroXsIsotope(map<string,vector<double> >& constant, const std::vector<double>& sigma_t);

		double getAbsorptionProb(Energy& energy) const {
			return absorption_prob[energy.first];
		}

		double getFissionProb(Energy& energy) const {
			return fission_prob[energy.first];
		}

		/* Fission reaction */
		void fission(Particle& particle, Random& random) const {
			(*fission_reaction)(particle,random);
		}

		/* Just one scattering reaction (from the scattering matrix) */
		void scatter(Particle& particle, Random& random) const {
			(*scattering_reaction)(particle,random);
		}

		~MacroXsIsotope();
	};

	class MacroXs: public Material {

		/* Number of groups */
		size_t ngroups;
		/* Total cross section */
		std::vector<double> mfp;
		/* Just one isotope */
		MacroXsIsotope* isotope;
	public:

		/* Name of this object */
		static std::string name() {return "macro-xs";}

		MacroXs(const MacroXsObject* definition, int number_groups);

		 /* Get the total cross section (using the energy index of the particle) */
		double getMeanFreePath(Energy& energy) const {return mfp[energy.first];};

		/* Macro-XS materials only have one "isotope" */
		const Isotope* getIsotope(Energy& energy, Random& random) const {return isotope;};

		/* Print material information */
		void print(std::ostream& out) const;

		/* Get number of groups */
		int getGroups() const {return ngroups;}

		virtual ~MacroXs();
	};

	/* Definition of a macroscopic cross section */
	class MacroXsObject : public MaterialObject {
		/* Map of macroscopic XS name to a vector of group constant */
		std::map<std::string,std::vector<double> > constant;
		friend class MacroXs;
	public:
		MacroXsObject(const MaterialId& matid, std::map<std::string,std::vector<double> >& constant) :
			MaterialObject(MacroXs::name(),matid), constant(constant) {/* */}

		std::map<std::string, std::vector<double> > getConstant() const {
			return constant;
		}
		~MacroXsObject() {/* */}
	};

	/* Material Factory */
	class MacroXsFactory : public MaterialFactory {
	public:
		/* Prevent construction or copy */
		MacroXsFactory() {/* */};
		/* Create a new materials */
		virtual std::vector<Material*> createMaterials(const std::vector<MaterialObject*>& definitions) const;
		virtual ~MacroXsFactory() {/* */}
	};

} /* namespace Helios */
#endif /* MACROXS_HPP_ */
