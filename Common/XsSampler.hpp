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

#ifndef XSSAMPLER_HPP_
#define XSSAMPLER_HPP_


#include <algorithm>
#include <vector>
#include <cassert>

namespace Helios {

	/*
	 * Generic class to sample objects with probabilities defined by a cross section. This class avoids
	 * storing zeroes for reactions with large energy thresholds.
	 */
	template<class TypeReaction>
	class XsSampler {
		typedef std::pair<TypeReaction,Ace::CrossSection*> XsData;

	protected:

		/* Dimension of the matrix */
		int nreaction;
		/* Smallest index on the grid */
		int emin;

		/* Container of reactions */
		std::vector<TypeReaction> reactions;

		/*
		 * How reactions are specified. XS are ordered from biggest to smallest index
		 *
		 *   ---------> Accumulated probability (or XS) for each reaction
		 * |        [r-0] [r-1] [r-2] [r-3] .... [r-n]
		 * | [e-m0]   -     -     -    0.5  ....  0.98
		 * | [e-m1]   -    0.3   0.45  0.6  ....  0.98
		 * | [e-m2]  0.3   0.4   0.55  0.7  ....  0.98
		 * |  ...
		 * | [e-mn]  0.4   0.5   0.65  0.8  ....  0.98
		 *
		 * e-mn is the energy index of the particle (not the value). e-m0 (emin) is the smallest index from all
		 * input cross section arrays.
		 */
		double* reaction_matrix;

		/* Get the index of the reaction after a binary search */
		int getIndex(const double* dat, double val);

	public:

		/* Functor to sort the cross sections */
		class CompareXs {
			bool operator() (const XsData& i, const XsData& j) {
				return (i.second->getIndex() < j.second->getIndex());
			}
		};

		/*
		 * How reactions are specified in the constructor (with an energy dependent table)
		 *
		 * There is a container of a pair that contains a reaction object and a pointer to a cross
		 * section array.
		 */
		XsSampler(const std::vector<XsData>& reactions) :
            nreaction(reactions.size()), reactions(nreaction) {

		}

		/*
		 * Sample a reaction
		 * index : row on the reaction matrix
		 * value : number between xs_min and xs_max to sample a reaction. If the XS table
		 * is normalized, xs_min = 0.0 and xs_max = 1.0
		 */
		TypeReaction sample(int index, double value);

		/* Get reaction container */
		const std::vector<TypeReaction>& getReactions() const {return reactions;}

		~Sampler() {delete [] reaction_matrix;};
	};

	template<class TypeReaction>
	int XsSampler<TypeReaction>::getIndex(const double* dat, double val) {
		/* Initial boundaries */
		const double* lo = dat;
		const double* hi = dat + (nreaction - 2);
		if(val < *lo) return 0;
		if(val > *hi) return nreaction - 1;
		const double* value = std::lower_bound(lo, hi + 1, val);
		return value - lo;
	}

	template<class TypeReaction>
	TypeReaction XsSampler<TypeReaction>::sample(int index, double value) {
		if(nreaction == 1) return reactions[0];
		int nrea = getIndex(reaction_matrix + index * (nreaction - 1), value);
		return reactions[nrea];
	}

} /* namespace Helios */



#endif /* XSSAMPLER_HPP_ */
