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

	struct TabularDistribution {

		int iflag;                 /* 1 = histogram, 2 = lin-lin */
		std::vector<double> out;   /* Outgoing values grid */
		std::vector<double> pdf;   /* Probability density function */
		std::vector<double> cdf;   /* Cumulative density function */

		TabularDistribution(int iflag, const std::vector<double>& out, const std::vector<double>& pdf, const std::vector<double>& cdf) :
			 iflag(iflag) ,out(out),pdf(pdf), cdf(cdf)
		{
			/* Sanity check */
			assert(out.size() == pdf.size());
			assert(cdf.size() == pdf.size());
		}

		double operator()(Random& random) const {
			/* Get random number */
			double chi = random.uniform();
			/* Sample the bin on the cumulative */
			size_t idx = std::upper_bound(cdf.begin(), cdf.end(), chi) - cdf.begin() - 1;
			/* Return outgoing value */
			return getOutgoing(chi, idx);
		}

		double operator()(Random& random, size_t& idx) const {
			/* Get random number */
			double chi = random.uniform();
			/* Sample the bin on the cumulative */
			idx = std::upper_bound(cdf.begin(), cdf.end(), chi) - cdf.begin() - 1;
			/* Return outgoing value */
			return getOutgoing(chi, idx);
		}

		double operator()(double chi, size_t& idx) const {
			/* Sample the bin on the cumulative */
			idx = std::upper_bound(cdf.begin(), cdf.end(), chi) - cdf.begin() - 1;
			/* Return outgoing value */
			return getOutgoing(chi, idx);
		}

		void print(std::ostream& sout) const {
			sout << setw(15) << "out" << setw(15) << "pdf" << setw(15) << "cdf" << endl;
			for(size_t i = 0 ; i < out.size() ; ++i)
				sout << scientific << setw(15) << out[i] << setw(15) << pdf[i] << setw(15) << cdf[i] << endl;
		}

		virtual ~TabularDistribution() {/* */}

	private:
		/* Return outgoing value */
		double getOutgoing(double chi, size_t idx) const {
			/* Histogram interpolation */
			if(iflag == 1) {
				/* Return cosine */
				return out[idx] + (chi - cdf[idx]) / pdf[idx];
			/* Linear-Linear interpolation */
			} else if(iflag == 2) {
				/* Auxiliary variables */
				double g = (pdf[idx + 1] - pdf[idx]) / (out[idx + 1] - out[idx]);
				double h = sqrt(pdf[idx] * pdf[idx] + 2*g*(chi - cdf[idx]));
				/* Solve for cosine */
				if(g == 0.0)
					/* Just like the histogram distribution */
					return out[idx] + (chi - cdf[idx]) / pdf[idx];
				else
					/* Interpolation */
					return out[idx] + (1/g) * (h - pdf[idx]);
			}
			return 0.0;
		}
	};

	/*
	 * Class to sample a table using an energy grid. Table's *kind* is
	 * arbitrary and should be specified as a template parameter.
	 */
	template<class TableType>
	class TableSampler {
		/* Make the interpolation using the data on the pair*/
		TableType getTable(double energy, Random& random, const std::pair<size_t,double>& res) const {
			/* Index */
			size_t idx = res.first;
			/* Interpolation factor */
			double factor = res.second;
			/* Sample bin and return the table */
			double chi = random.uniform();
			if(chi < factor) return tables[idx + 1];
			return tables[idx];
		}
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
			/* Get table */
			return getTable(energy, random, res);
		}
		/* Sample table and set interpolation data */
		TableType sample(double energy, Random& random, std::pair<size_t,double>& res) const {
			/* Get interpolation data */
			res = interpolate(energies.begin(), energies.end(), energy);
			/* Get table */
			return getTable(energy, random, res);
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

	/*
	 * Policies to make the change from LAB-LAB and CM-LAB reference systems
	 */

	/* Make the change from CM to LAB system */
	class CenterOfMass {
		/* Atomic weight ratio */
		double awr;
	public:
		CenterOfMass(double awr) : awr(awr) {/* */}
		/* Make the transformation from CM to LAB system */
		void transform(const Particle& particle, double& energy, double& mu) const {
			/* Outgoing energy on the CM system */
			double ecm = energy;
			/* Incident particle energy on LAB system */
			double ein = particle.getEnergy().second;
			/* Factor */
			double ac = awr + 1.0;
			/* Energy from CM to LAB frame (using MU sampled on CM frame) */
			energy = ecm + (ein + 2.0 * mu*ac*sqrt(ein*ecm))/(ac*ac);
			/* Cosine from CM to LAB frame (using energy on LAB frame) */
			mu = mu * sqrt(ecm/energy) + sqrt(ein/energy)/ac;
		}
		~CenterOfMass(){/* */}
	};

	/* The sampling was on LAB frame, nothing should be done */
	class Laboratory {
	public:
		Laboratory(double awr) {/* */}
		void transform(const Particle& particle, double& energy, double& mu) const {/* */}
		~Laboratory(){/* */}
	};

}

}

#endif /* ACEREACTIONCOMMON_HPP_ */
