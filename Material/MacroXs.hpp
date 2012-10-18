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

#include "Material.hpp"
#include "../Common/Sampler.hpp"

namespace Helios {

class MacroXs: public Helios::Material {

	/* Number of groups */
	int ngroups;
	/* Total cross section */
	std::vector<double> mfp;

	/* ---- Reactions related to macroscopic cross sections */

	class Absorption : public Reaction {
	public:
		Absorption() {/* */}
		void operator() (Particle& particle, Random& r) const {particle.sta() = Particle::DEAD;};
		~Absorption() {/* */}
	};

	class Fission : public Reaction {
		/* NU value for each group */
		std::vector<double> nu;
		/* Spectrum sampler */
		Sampler<int>* spectrum;
	public:
		Fission(const std::vector<double>& nu, const std::vector<double>& chi) : nu(nu) {
			/* Set map for the spectrum sampler */
			std::map<int,double> m;
			for(size_t i = 0 ; i < chi.size() ; ++i)
				m[i] = chi[i];
			spectrum = new Sampler<int>(m);
		}
		void operator() (Particle& particle, Random& r) const {
			/* The state should be banked */
			particle.sta() = Particle::BANK;
			/* Get number of particles */
			double nubar = nu[particle.eix()];
			/* Integer part */
			int nu = (int) nubar;
			if (r.uniform() < nubar - (double)nu)
				nu++;
			particle.wgt() *= (double)nu;
			/* New direction */
			isotropicDirection(particle.dir(),r);
			/* New group */
			particle.eix() = spectrum->sample(0,r.uniform());
		};
		~Fission() {
			delete spectrum;
		}
	};

	class Scattering : public Reaction {
		/* Spectrum sampler */
		Sampler<int>* spectrum;
	public:
		Scattering(const std::vector<double>& sigma_scat, size_t ngroups) {
			/* Set map for the spectrum sampler, we got the scattering matrix */
			std::map<int,std::vector<double> > m;
			std::vector<double> v(ngroups);
			for(size_t i = 0 ; i < ngroups ; ++i) {
				for(size_t j = 0 ; j < ngroups ; ++j)
					v[j] = sigma_scat[j * ngroups + i];
				m[i] = v;
			}
			spectrum = new Sampler<int>(m);
		}
		void operator() (Particle& particle, Random& r) const {
			/* New direction */
			isotropicDirection(particle.dir(),r);
			/* New group */
			particle.eix() = spectrum->sample(particle.eix(),r.uniform());
		};
		~Scattering() {
			delete spectrum;
		}
	};

	/* Reaction sampler */
	Sampler<Reaction*>* reaction_sampler;

public:

	/* Definition of a macroscopic cross section */
	class Definition : public Material::Definition {
		/* Map of macroscopic XS name to a vector of group constant */
		std::map<std::string,std::vector<double> > constant;
	public:
		Definition(const MaterialId& matid, std::map<std::string,std::vector<double> >& constant) :
				   Material::Definition("macro-xs",matid), constant(constant) {/* */}

		std::map<std::string, std::vector<double> > getConstant() const {
			return constant;
		}
	};

	MacroXs(const Material::Definition* definition, int number_groups);

	/*
	 * Based on particle's energy, this functions setup the index on the energy grid with
	 * information contained on the child class.
	 */
	EnergyIndex getEnergyIndex(const Energy& energy) const {return 0;};

	 /* Get the total cross section (using the energy index of the particle) */
	double getMeanFreePath(const EnergyIndex& index) const {return mfp[index];};

	/* Get reaction (based on a random generator and a energy index) */
	Reaction* getReaction(const EnergyIndex& index, Random& random) const {
		double value = random.uniform();
		return reaction_sampler->sample(index,value);
	}

	/* Print material information */
	void print(std::ostream& out) const;

	virtual ~MacroXs();
};

} /* namespace Helios */
#endif /* MACROXS_HPP_ */
