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
#include <tbb/cache_aligned_allocator.h>

namespace Helios {

	/*
	 * Generic class to sample objects with probabilities defined by a cross section. This class avoids
	 * storing zeroes for reactions with large energy thresholds.
	 */
	template<class TypeReaction>
	class XsSampler {
		typedef std::pair<TypeReaction,const Ace::CrossSection*> XsData;

	protected:

		/* Dimension of the matrix */
		int nreaction;
		/* Smallest index on the grid */
		int emin;
		/* Number of energies (total, not stored) */
		int nenergy;

		/* Container of reactions */
		std::vector<TypeReaction> reactions;

		/*
		 * Default value to be returned in case the energy index is out of range (lower than emin).
		 */
		TypeReaction default_reaction;

		/* Offset for reactions different from zero at each energy */
		std::vector<int,tbb::cache_aligned_allocator<int> > offsets;

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
		std::vector<std::vector<double,tbb::cache_aligned_allocator<double> > > reaction_matrix;

		/* Get the index of the reaction after a binary search */
		size_t getIndex(size_t nrow, double val, double factor);

		/* Get a value on the matrix */
		double getMatrixValue(size_t nerg, size_t nrea) {
			/* Get row */
			std::vector<double, tbb::cache_aligned_allocator<double> >& row = reaction_matrix[nerg];
			/* Get local coordinate */
			int li = nrea - ((nreaction - 1) - row.size());
			/* Check out of range index */
			if(li < 0) return 0.0;
			else return row[li];
		}

		/* Interpolate a value between two energies on the grid */
		double intepolateMatrixValue(size_t nerg, size_t nrea, double factor) {
			/* Get limits */
			double low_value = getMatrixValue(nerg,nrea);
			double high_value = getMatrixValue(nerg + 1,nrea);
			/* Apply the interpolation factor */
			return low_value + factor * (high_value - low_value);
		}

		/* Extension of the STL lower_bound algorithm using a functor for evaluation */
		size_t reaction_lower_bound(size_t nrow, double val, double factor) {
			/* Low row */
			std::vector<double,tbb::cache_aligned_allocator<double> >& low = reaction_matrix[nrow];
			/* High row */
			std::vector<double,tbb::cache_aligned_allocator<double> >& high = reaction_matrix[nrow + 1];

			/* Get lower reaction index */
			size_t first = ((nreaction - 1) - std::max(low.size(), high.size()));
			/* Last reaction index */
			size_t last = nreaction - 1;
			/* Initial length */
			size_t len = last - first;
			size_t half, middle;

			/* Binary search */
			while (len > 0) {
				half = len >> 1;
				middle = first;
				middle += half;
				/* Get interpolated value */
				if (intepolateMatrixValue(nrow, middle, factor) < val) {
					first = middle;
					++first;
					len = len - half - 1;
				} else len = half;
			}
			return first;
		}

	public:

		/* Functor to sort the cross sections */
		struct CompareXs {
			bool operator() (const XsData& i, const XsData& j) {
				return (i.second->getIndex() > j.second->getIndex());
			}
		};

		/*
		 * Constructor
		 *
		 * There is a container of pairs that contains a reaction object and a pointer to a cross
		 * section array.
		 */
		XsSampler(const std::vector<XsData>& _reas) :
            nreaction(_reas.size()) {

			/* Copy reactions (to sort the XS) */
			std::vector<XsData> reas = _reas;

			/* Check number of reaction */
			if(nreaction == 1) {
				/* Push reaction, and return */
				reactions.push_back(reas.begin()->first);
				return;
			}

			/* Sort the array (from highest index to lowest)*/
			std::sort(reas.begin(), reas.end(), CompareXs());

			/* Set the smallest index on the grid */
			emin = (*(reas.end() - 2)).second->getIndex() - 1;

			/* Set the default value */
			default_reaction = (*(reas.end() - 1)).first;

			/* Set number of energies */
			nenergy = (*reas.begin()).second->size();

			/* Initialize the offset array */
			offsets.resize(nenergy - emin);

			/* Initialize reaction matrix */
			reaction_matrix.resize(nenergy - emin);

			for(size_t i = 0 ; i < offsets.size() ; ++i) {
				offsets[i] = 0;
				/* Loop to calculate offsets */
				for(typename std::vector<XsData>::const_iterator it = reas.begin() ; it != reas.end() ; ++it) {
					/* Get index on this reaction */
					int index = (*it).second->getIndex() - 1;
					/* Accumulate if reaction is not outside of the range */
					if(index <= (emin + (int)i))
						offsets[i]++;
					/* Push reactions (in an ordered way) only one time */
					if(i == 0)
						reactions.push_back((*it).first);
				}
			}

			/* Loop over energies */
			for(size_t i = 0 ; i < offsets.size() ; ++i) {
				/* Get row */
				std::vector<double,tbb::cache_aligned_allocator<double> >& row = reaction_matrix[i];
				/* Resize matrix row */
				row.resize(offsets[i] - 1);
				/* Get absolute value of energy */
				int nerg = emin + (int)i;
				/* Calculate partial sums */
				double partial_sum = 0.0;
				for(size_t j = 0 ; j < row.size(); ++j) {
					/* Get absolute index */
					size_t rea = j + ((nreaction - 1) - row.size());
					/* Cross section */
					const Ace::CrossSection* xs = reas[rea].second;
					/* Accumulate partial sum */
					partial_sum += (*xs)[nerg];
					/* Set value on the matrix */
					row[j] = partial_sum;
				}
			}
		}

		/*
		 * Sample a reaction
		 * index : row on the reaction matrix
		 * value : number between xs_min and xs_max to sample a reaction. If the XS table
		 * is normalized, xs_min = 0.0 and xs_max = 1.0. <factor> is used to get an interpolated
		 * value when doing the binary search.
		 */
		TypeReaction sample(int index, double value, double factor);

		/* Get reaction container */
		const std::vector<TypeReaction>& getReactions() const {return reactions;}

		~XsSampler() {/* */};
	};

	template<class TypeReaction>
	size_t XsSampler<TypeReaction>::getIndex(size_t nrow, double val, double factor) {
		/* Initial boundaries */
		if(val < intepolateMatrixValue(nrow, 0, factor)) return 0;
		if(val > intepolateMatrixValue(nrow, nreaction - 2, factor)) return nreaction - 1;
		return reaction_lower_bound(nrow, val, factor);
	}

	template<class TypeReaction>
	TypeReaction XsSampler<TypeReaction>::sample(int index, double value, double factor) {
		/* Check number of reactions */
		if(nreaction == 1) return reactions[0];
		/* Check index */
		if(index < emin) return default_reaction;
		size_t nrea = getIndex(index - emin, value, factor);
		return reactions[nrea];
	}

} /* namespace Helios */

#endif /* XSSAMPLER_HPP_ */
