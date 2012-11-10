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

#ifndef ACEREACTIONCOMMON_HPP_
#define ACEREACTIONCOMMON_HPP_

#include <algorithm>

#include "../AceModule.hpp"
#include "../../../Common/Interpolate.hpp"

namespace Helios {

namespace AceReaction {

	/* Common classes / templates for ACE reactions */
	class TabularDistribution {
		int iflag;                 /* 1 = histogram, 2 = lin-lin */
		std::vector<double> csout; /* Cosine scattering angular grid */
		std::vector<double> pdf;   /* Probability density function */
		std::vector<double> cdf;   /* Cumulative density function */
	public:
		TabularDistribution(int iflag, const std::vector<double>& csout, const std::vector<double>& pdf, const std::vector<double>& cdf) :
			 iflag(iflag) ,csout(csout),pdf(pdf), cdf(cdf)
		{
			/* Sanity check */
			assert(csout.size() == pdf.size());
			assert(cdf.size() == pdf.size());
		}

		double operator()(Random& random) const {
			/* Get random number */
			double chi = random.uniform();
			/* Sample the bin on the cumulative */
			size_t idx = std::upper_bound(cdf.begin(), cdf.end(), chi) - cdf.begin() - 1;

			/* Histogram interpolation */
			if(iflag == 1) {
				/* Return cosine */
				return csout[idx] + (chi - cdf[idx]) / pdf[idx];
			/* Linear-Linear interpolation */
			} else if(iflag == 2) {
				/* Auxiliary variables */
				double g = (pdf[idx + 1] - pdf[idx]) / (csout[idx + 1] - csout[idx]);
				double h = sqrt(pdf[idx] * pdf[idx] + 2*g*(chi - cdf[idx]));
				/* Solve for cosine */
				if(g == 0.0)
					/* Just like the histogram distribution */
					return csout[idx] + (chi - cdf[idx]) / pdf[idx];
				else
					/* Interpolation */
					return csout[idx] + (1/g) * (h - pdf[idx]);
			}

			return 0.0;
		}

		void print(std::ostream& out) const {
			out << setw(15) << "out" << setw(15) << "pdf" << setw(15) << "cdf" << endl;
			for(size_t i = 0 ; i < csout.size() ; ++i)
				out << scientific << setw(15) << csout[i] << setw(15) << pdf[i] << setw(15) << cdf[i] << endl;
		}

		virtual ~TabularDistribution() {/* */}
	};

	/*
	 * Class to sample a table using an energy grid. Table's *kind* is
	 * arbitrary and should be specified as a template parameter.
	 */
	template<class TableType>
	class TableSampler {
	protected:
		/* A table contains an energy grid... */
		std::vector<double> energies;
		/* ... and a Table container */
		std::vector<TableType> tables;
	public:
		TableSampler() {/* */}
		/* Sample table */
		TableType sample(double energy, Random& random) const {
			/* Get interpolation data */
			std::pair<size_t,double> res = interpolate(energies.begin(), energies.end(), energy);
			/* Index */
			size_t idx = res.first;
			/* Interpolation factor */
			double factor = res.second;
			/* Sample bin and return the table */
			double chi = random.uniform();
			if(chi < factor) return tables[idx + 1];
			return tables[idx];
		}
		/* Get a pair energy/table for some index */
		std::pair<double, TableType> operator[](size_t idx) const {
			return std::pair<double, TableType>(energies[idx], tables[idx]);
		}
		/* Get size */
		size_t size() const {
			return energies.size();
		}
		virtual ~TableSampler() {/* */}
	};
}

}

#endif /* ACEREACTIONCOMMON_HPP_ */
