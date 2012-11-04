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

#ifndef FACTORSAMPLER_HPP_
#define FACTORSAMPLER_HPP_

#include "Sampler.hpp"

namespace Helios {

	/* Extension of the STL lower_bound algorithm using a functor for evaluation */
	template<typename Iterator, typename Tp, typename Evaluate>
	Iterator eval_lower_bound(Iterator first, Iterator last, const Tp& val, Evaluate& eval) {
		typedef typename iterator_traits<Iterator>::value_type ValueType;
		typedef typename iterator_traits<Iterator>::difference_type DistanceType;
		DistanceType len = std::distance(first, last);
		DistanceType half;
		Iterator middle;

		while (len > 0) {
		half = len >> 1;
		middle = first;
		std::advance(middle, half);
			if (eval(middle) < val) {
				first = middle;
				++first;
				len = len - half - 1;
			} else len = half;
		}
		return first;
	}

	/*
	 * Generic class to sample objects with probabilities defined by a cross section using an
	 * interpolation factor when performing the binary search.
	 */
	template<class TypeReaction>
	class FactorSampler : public Sampler<TypeReaction> {

		/* Get the index of the reaction after a binary search */
		int getIndex(const double* dat, double val, double factor);

		/*
		 * How reactions are specified
		 *
		 *   ---------> Accumulated probability (or XS) for each reaction
		 * |       [r-0] [r-1] [r-2] [r-3] .... [r-n]
		 * | [e-0]  0.1   0.2   0.35  0.5  ....  0.98
		 * | [e-1]  0.2   0.3   0.45  0.6  ....  0.98
		 * | [e-2]  0.3   0.4   0.55  0.7  ....  0.98
		 * |  ...
		 * | [e-n]  0.4   0.5   0.65  0.8  ....  0.98
		 *
		 * e-n is the energy index of the particle (not the value)
		 *
		 * When using the FactorSampler, an interpolation factor should be specified before
		 * sampling the reaction. For example, if the energy is between e-1 and e-2, with an
		 * interpolation factor equal to 0.5, the sampling is equivalent to:
		 *
		 * | [e-1]  0.20  0.30  0.45  0.60  ....  0.98
		 *           .     .     .     .           .
		 * | [e]--> 0.25  0.35  0.50  0.65        0.98
 		 *           .     .     .     .           .
		 * | [e-2]  0.30  0.45  0.55  0.70  ....  0.98
		 *
		 * The sample method takes as an argument the lowest index (e-1) and the interpolation
		 * factor (=0.50). This effect is accomplished combining the eval_lower_bound algorithm
		 * and the Interpolate functor.
		 *
		 */

	public:

		/* Functor used to get an interpolated value on the reaction matrix (given an interpolation factor) */
		class Interpolate {
			size_t stride;
			double factor;
		public:
			Interpolate(size_t stride, double factor) : stride(stride), factor(factor) {/* */}
			double operator()(const double* ptr) {
				double min = *ptr;
				double max = *(ptr + stride);
				return factor * (max - min) + min;
			}
 			~Interpolate() {/* */}
		};

		/*
		 * How reactions are specified in the constructor (with an energy dependent table)
		 *
		 * The reactions are keys on the map, and the value associated to each key is some kind
		 * of container to the probabilities in function of the energy.
		 *
		 *   ---------> Cross section for each
		 * |       [e-0] [e-1] [e-2] [e-3] .... [e-n]
		 * | [r-0]  0.1   0.1   0.35  0.5  ....  0.2
		 * | [r-1]  0.2   0.3   0.45  0.6  ....  0.4
		 * | [r-2]  0.3   0.4   0.55  0.7  ....  0.6
		 * |  ...
		 * | [r-n]  0.4   0.5   0.65  0.8  ....  0.7
		 */

		template<class ProbTable>
		FactorSampler(const std::map<TypeReaction,ProbTable>& reaction_map) : Sampler<TypeReaction>(reaction_map) {/* */}

		template<class ProbTable>
		FactorSampler(const std::vector<TypeReaction>& reactions, const std::vector<ProbTable>& xs_container, bool normalize = true) :
		Sampler<TypeReaction>(reactions,xs_container,normalize) {/* */}

		template<class ProbTable>
		FactorSampler(const std::vector<TypeReaction>& reactions,const std::vector<ProbTable>& xs_container,const ProbTable& total_xs) :
		Sampler<TypeReaction>(reactions,xs_container,total_xs) {/* */}

		/*
		 * Sample a reaction
		 * index : row on the reaction matrix
		 * value : number between xs_min and xs_max to sample a reaction. If the XS table
		 * is normalized, xs_min = 0.0 and xs_max = 1.0. <factor> is used to get an interpolated
		 * value when doing the binary search.
		 */
		TypeReaction sample(int index, double value, double factor);

	};

	template<class TypeReaction>
	int FactorSampler<TypeReaction>::getIndex(const double* dat, double val, double factor) {
		/* Create interpolation object */
		Interpolate interpolator(Sampler<TypeReaction>::nreaction - 1, factor);
		/* Initial boundaries */
		const double* lo = dat;
		const double* hi = dat + (Sampler<TypeReaction>::nreaction - 2);
		if(val < interpolator(lo)) return 0;
		if(val > interpolator(hi)) return Sampler<TypeReaction>::nreaction - 1;
		const double* value = eval_lower_bound(lo, hi + 1, val, interpolator);
		return value - lo;
	}

	template<class TypeReaction>
	TypeReaction FactorSampler<TypeReaction>::sample(int index, double value, double factor) {
		if(Sampler<TypeReaction>::nreaction == 1) return Sampler<TypeReaction>::reactions[0];
		int nrea = getIndex(Sampler<TypeReaction>::reaction_matrix + index * (Sampler<TypeReaction>::nreaction - 1), value, factor);
		return Sampler<TypeReaction>::reactions[nrea];
	}
}

#endif /* FACTORSAMPLER_HPP_ */
