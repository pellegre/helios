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

#ifndef SAMPLER_HPP_
#define SAMPLER_HPP_

#include <algorithm>
#include <vector>
#include <cassert>

namespace Helios {

	template<class TypeReaction>

	class Sampler {

		/* Dimension of the matrix */
		int nreaction;
		int nenergy;

		/* Container of reactions */
		std::vector<TypeReaction> reactions;

		/*
		 * How reactions are specified
		 *
		 *   ---------> Accumulated probability for each reaction
		 * |       [r-0] [r-1] [r-2] [r-3] .... [r-n]
		 * | [e-0]  0.1   0.2   0.35  0.5  ....  0.98
		 * | [e-1]  0.2   0.3   0.45  0.6  ....  0.98
		 * | [e-2]  0.3   0.4   0.55  0.7  ....  0.98
		 * |  ...
		 * | [e-n]  0.4   0.5   0.65  0.8  ....  0.98
		 *
		 * e-n is the energy index of the particle (not the value)
		 */
		double* reaction_matrix;

		/* Get the index of the reaction after a binary search */
		int getIndex(const double* dat, double val);

		/* Get value in an index from different containers types */
		double getArrayIndex(int index,const std::vector<double> stl_array) {
			return stl_array[index];
		}
		double getArrayIndex(int index,const std::vector<double>* stl_array_ptr) {
			return stl_array_ptr->at(index);
		}
		/* In case is not a container (just a value) */
		double getArrayIndex(int index,const double single_value) {
			assert(index == 0);
			return single_value;
		}

		/* Get size different containers types */
		double getArraySize(const std::vector<double> stl_array) {
			return stl_array.size();
		}
		double getArraySize(const std::vector<double>* stl_array_ptr) {
			return stl_array_ptr->size();
		}
		/* In case is not a container (just a value) */
		double getArraySize(const double single_value) {
			return 1;
		}

	public:

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
		 *
		 */

		/*
		 * How reactions are specified in the constructor (in case there isn't and energy dependent table)
		 *
		 * The reactions are keys on the map, and the value associated to each key is the
		 * probability of that reaction
		 *
		 *   ---------> Cross section for each
		 * |       [value]
		 * | [r-0]   0.1
		 * | [r-1]   0.2
		 * | [r-2]   0.3
		 * |  ...
		 * | [r-n]   0.4
		 *
		 */

		template<class ProbTable>
		Sampler(const std::map<TypeReaction,ProbTable>& reaction_map) :
            nreaction(reaction_map.size()),
            nenergy(getArraySize(reaction_map.begin()->second)),
            reactions(nreaction) {

			/* Allocate reaction matrix */
			reaction_matrix = new double[(nreaction - 1) * nenergy];

			/* TypeReactions */
			typename std::map<TypeReaction,ProbTable>::const_iterator it_rea = reaction_map.begin();
			int nrea = 0;
			/* Cross sections */
			std::vector<ProbTable> xs_container(nreaction);
			for(; it_rea != reaction_map.end() ; ++it_rea) {
				/* Save the reactions into the reaction container */
				reactions[nrea] = (*it_rea).first;
				/* Save the cross sections */
				xs_container[nrea] = (*it_rea).second;
				/* Count reaction */
				nrea++;
			}

			/* Once we separate the reactions from the cross sections, we need to construct the reaction matrix */
			for(int nerg = 0 ; nerg < nenergy ; ++nerg) {
				/* First get the total cross section (of this reactions) at this energy */
				double total_xs = 0.0;
				for(nrea = 0 ; nrea < nreaction ; ++nrea)
					total_xs += getArrayIndex(nerg,xs_container[nrea]);

				/* Exclusive scan, to construct the accumulated probability table at this energy */
				double partial_sum = 0;
				for(nrea = 0 ; nrea < nreaction - 1; ++nrea) {
					partial_sum += getArrayIndex(nerg,xs_container[nrea]);
					reaction_matrix[nerg*(nreaction - 1) + nrea] = partial_sum / total_xs;
				}
			}
		}

		template<class ProbTable>
		Sampler(const std::vector<TypeReaction>& reactions, const std::vector<ProbTable>& xs_container) :
            nreaction(reactions.size()),
            nenergy(getArraySize(*xs_container.begin())),
            reactions(reactions) {

			/* Allocate reaction matrix */
			reaction_matrix = new double[(nreaction - 1) * nenergy];

			/* Sanity check */
			assert(xs_container.size() == reactions.size());

			/* Once we separate the reactions from the cross sections, we need to construct the reaction matrix */
			for(int nerg = 0 ; nerg < nenergy ; ++nerg) {
				/* First get the total cross section (of this reactions) at this energy */
				double total_xs = 0.0;
				for(int nrea = 0 ; nrea < nreaction ; ++nrea)
					total_xs += getArrayIndex(nerg,xs_container[nrea]);

				/* Exclusive scan, to construct the accumulated probability table at this energy */
				double partial_sum = 0;
				for(int nrea = 0 ; nrea < nreaction - 1; ++nrea) {
					partial_sum += getArrayIndex(nerg,xs_container[nrea]);
					reaction_matrix[nerg*(nreaction - 1) + nrea] = partial_sum / total_xs;
				}
			}
		}

		/*
		 * Sample a reaction
		 * index : row on the reaction matrix
		 * value : number between 0.0 and 1.0 to sample a reaction.
		 */
		TypeReaction sample(int index, double value);

		/* Get reaction container */
		const std::vector<TypeReaction>& getReactions() const {return reactions;}

		~Sampler() {delete [] reaction_matrix;};

	};

	template<class TypeReaction>
	int Sampler<TypeReaction>::getIndex(const double* dat, double val) {
		/* Initial boundaries */
		const double* lo = dat;
		const double* hi = dat + (nreaction - 2);
		if(val < *lo) return 0;
		if(val > *hi) return nreaction - 1;
		const double* value = std::lower_bound(lo,hi + 1,val);
		return value - lo;
	}

	template<class TypeReaction>
	TypeReaction Sampler<TypeReaction>::sample(int index, double value) {
		if(nreaction == 1) return reactions[0];
		int nrea = getIndex(reaction_matrix + index * (nreaction - 1),value);
		return reactions[nrea];
	}

} /* namespace Helios */
#endif /* SAMPLER_HPP_ */
