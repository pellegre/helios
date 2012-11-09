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

#ifndef MUSAMPLER_HPP_
#define MUSAMPLER_HPP_

#include <vector>
#include <algorithm>
#include <cassert>

#include "../../../Common/Common.hpp"
#include "../../../Common/Interpolate.hpp"
#include "../../../Transport/Particle.hpp"
#include "../AceReader/AngularDistribution.hpp"
#include "../AceReader/NeutronReaction.hpp"

#include "AceReactionCommon.hpp"

namespace Helios {

namespace AceReaction {

	/* Tables for sampling scattering cosine obtained from an ACE cross section library */
	typedef Ace::AngularDistribution::AngularArray AceAngular;
	typedef Ace::AngularDistribution::EquiBins AceEquiBins;
	typedef Ace::AngularDistribution::Tabular AceTabular;

	/* Base sampling table */
	class CosineTable {
	public:
		CosineTable() {/* */}
		virtual double operator()(Random& random) const = 0;
		virtual void print(std::ostream& out) const = 0;
		virtual ~CosineTable() {/* */}
	};

	/* Sample isotropic scattering cosine */
	class Isotropic : public CosineTable {
	public:
		Isotropic() {/* */}

		double operator()(Random& random) const {
			/* Return value */
			return (1.0 - 2.0 * random.uniform());
		}

		void print(std::ostream& out) const {
			out << " * Isotropic distribution (no information available) " << endl;
		}

		~Isotropic() {/* */}
	};

	/* Sample scattering cosine using 32 equiprobable bins */
	class EquiBins : public CosineTable {
		/* Cosine bins */
		std::vector<double> bins;
	public:
		EquiBins(const AceEquiBins* ace_angular) :
			bins(ace_angular->bins)
		{
			/* Sanity check */
			assert(bins.size() == 33);
		}

		double operator()(Random& random) const {
			/* Sample random number */
			double chi = random.uniform();
			/* Sample bin */
			size_t pos = (size_t) (chi * 32);
			/* Get interpolated cosine */
			return bins[pos] + (chi - pos) * (bins[pos + 1] - bins[pos]);
		}

		void print(std::ostream& out) const {
			out << " * 32 Equiprobable Cosine bins " << endl;
			for(size_t i = 0 ; i < bins.size() ; ++i)
				out << scientific << setw(15) << bins[i] << endl;
		}

		~EquiBins() {/* */}
	};

	/* Sample scattering cosine using a tabular distribution */
	class Tabular : public TabularDistribution, public CosineTable /* defined on AceReactionCommon.hpp */ {
	public:

		Tabular(const AceTabular* ace_angular) :
			TabularDistribution(ace_angular->iflag, ace_angular->csout, ace_angular->pdf,ace_angular->cdf)
		{/* */}

		double operator()(Random& random) const {
			return TabularDistribution::operator()(random);
		}

		void print(std::ostream& out) const {
			out << " * 32 Equiprobable Cosine bins " << endl;
			TabularDistribution::print(out);
		}

		~Tabular() {/* */}
	};

	/*
	 * Base class to deal with cosine samplers
	 */
	class MuSampler {
	public:
		MuSampler(const Ace::AngularDistribution& ace_data) {/* */}
		/* Set the cosine with particle's information */
		virtual void setCosine(const Particle& particle, Random& random, double& mu) const = 0;
		/* Print internal data of the sampler */
		virtual void print(std::ostream& out) const = 0;
		virtual ~MuSampler() {/* */}
	};

	/*
	 * Cosine table sampler.
	 *
	 * This class have a sampler table for each incident energy tabulated.
	 * Before sampling a scattering cosine, the class samples the cosine table
	 * using the incident particle energy.
	 */
	class MuTable : public MuSampler {
		/* Tabulated incident energies */
		std::vector<double> energies;
		/* Cosine table for each tabulated energy */
		std::vector<CosineTable*> cosine_table;

		/* Cosine table builder */
		static CosineTable* tableBuilder(const AceAngular* ace_array);
	public:
		MuTable(const Ace::AngularDistribution& ace_data);

		/* Sample scattering cosine */
		void setCosine(const Particle& particle, Random& random, double& mu) const {
			/* Get particle energy */
			double energy = particle.getEnergy().second;
			/* Get interpolation data */
			std::pair<size_t,double> res = interpolate(energies.begin(), energies.end(), energy);
			/* Index */
			size_t idx = res.first;
			/* Interpolation factor */
			double factor = res.second;
			/* Sample bin and return cosine */
			double chi = random.uniform();
			if(chi < factor) mu = (*cosine_table[idx + 1])(random);
			else mu = (*cosine_table[idx])(random);
		}

		void print(std::ostream& out) const;

		~MuTable() {/* */}
	};

	/*
	 * Cosine isotropic sampler.
	 *
	 * In this case, no angular distribution data are given for this reaction,
	 * and isotropic scattering is assumed in either the LAB or CM system.
	 */
	class MuIsotropic : public MuSampler {
		Isotropic isotropic;
	public:
		MuIsotropic(const Ace::AngularDistribution& ace_data) : MuSampler(ace_data) {/* */};

		/* Sample scattering cosine */
		void setCosine(const Particle& particle, Random& random, double& mu) const {
			mu = isotropic(random);
		}

		void print(std::ostream& out) const {
			cout << " - Isotropic cosine sampler (no information available) " << endl;
		}

		~MuIsotropic() {/* */}
	};

}

}


#endif /* MUSAMPLER_HPP_ */
