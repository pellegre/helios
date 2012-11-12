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

#ifndef NUSAMPLER_HPP_
#define NUSAMPLER_HPP_

#include "../../../Common/Interpolate.hpp"
#include "../AceReader/TyrDistribution.hpp"

namespace Helios {

namespace AceReaction {
	/*
	 * Classes to sample the number of particles released on a reaction. Could be
	 * a fixed value or energy dependent.
	 */

	/*
	 * Base class to deal with NU sampling
	 */
	class NuSampler {
	protected:
		/* Get integer part of floating point sampled NU */
		double integerPart(double nubar, Random& random) const {
			/* Integer part */
			int nu = (int) nubar;
			if (random.uniform() < nubar - (double)nu)
				nu++;
			return nu;
		}
	public:
		NuSampler() {/* */}
		virtual double getNu(double energy, Random& random) const = 0;
		virtual void print(std::ostream& out) const = 0;
		virtual ~NuSampler() {/* */}
	};

	/* One outgoing particle */
	class OneNu : public NuSampler {
	public:
		OneNu(const Ace::TyrDistribution& tyr) {/* */}
		double getNu(double energy, Random& random) const {return 1;}
		void print(std::ostream& out) const {
			out << " * No NU (one outgoing particle)" << endl;
		}
		~OneNu() {/* */}
	};

	/* Fixed NU (No dependency with particle's energy) */
	class FixedNu : public NuSampler {
		double number;
	public:
		FixedNu(const Ace::TyrDistribution& tyr) : number(abs(tyr.getTyr())) {/* */}
		double getNu(double energy, Random& random) const {return number;}
		void print(std::ostream& out) const {
			out << " * Fixed NU = " << number << endl;
		}
		~FixedNu() {/* */}
	};

	/* Get NU from a table */
	class TabularNu : public NuSampler {
		std::vector<double> energies; /* tabular energies points */
		std::vector<double> nu;       /* Values of NU */
	public:
		/* Get from data on DLW block */
		TabularNu(const Ace::TyrDistribution& tyr) : energies(tyr.getEnergies()), nu(tyr.getNu()) {/* */}
		/* Get from data on NU block */
		TabularNu(const Ace::NUBlock::Tabular* nu_data) : energies(nu_data->energies), nu(nu_data->nu) {/* */}
		double getNu(double energy, Random& random) const {
			/* Get interpolation data */
			std::pair<size_t,double> res = interpolate(energies.begin(), energies.end(), energy);
			/* Index */
			size_t idx = res.first;
			/* Interpolation factor */
			double factor = res.second;
			/* Return interpolated NU */
			return integerPart(factor * (nu[idx + 1] - nu[idx]) + nu[idx], random);
		}
		void print(std::ostream& out) const {
			out << " * Tabular NU " << endl;
			for(size_t i = 0 ; i  < energies.size() ; ++i)
				out << scientific << setw(15) << energies[i] << setw(15) << nu[i] << endl;
		}
		~TabularNu() {/* */}
	};

	/* Get NU from a polynomial function */
	class PolynomialNu : public NuSampler {
		std::vector<double> coeffs;
	public:
		/* Get from data on NU block */
		PolynomialNu(const Ace::NUBlock::Polynomial* nu_data) : coeffs(nu_data->coef) {/* */}
		double getNu(double energy, Random& random) const {
			/* Initial energy */
			double erg = 1.0;
			/* Accumulated NU */
			double accum = 0.0;
			for(size_t i = 0 ; i < coeffs.size() ; ++i) {
				accum += coeffs[i] * erg;
				erg *= energy;
			}
			return integerPart(accum, random);
		}
		void print(std::ostream& out) const {
			out << " * Polynomial NU " << endl;
			for(size_t i = 0 ; i  < coeffs.size() ; ++i)
				out << scientific << coeffs[i] << " ";
			out << endl;
		}
		~PolynomialNu() {/* */}
	};
}

}


#endif /* NUSAMPLER_HPP_ */
