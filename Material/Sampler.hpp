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
		 * | [e-0]  0.1   0.2   0.35  0.5  ....  1.0
		 * | [e-1]  0.2   0.3   0.45  0.6  ....  1.0
		 * | [e-2]  0.3   0.4   0.55  0.7  ....  1.0
		 * |  ...
		 * | [e-n]  0.4   0.5   0.65  0.8  ....  1.0
		 *
		 * e-n is the energy index of the particle (not the value)
		 */
		Matrix reaction_matrix;

		/* Get the index of the reaction after a binary search */
		int getIndex(const Vector& dat, double val);

	public:

		Sampler(const std::map<TypeReaction,std::vector<double>* >& reaction_map);
		Sampler(const std::map<TypeReaction,std::vector<double> >& reaction_map);
		Sampler(const std::map<TypeReaction,double>& reaction_map);

		/*
		 * Sample a reaction
		 * index : row on the reaction matrix
		 * value : number between 0.0 and 1.0 to sample a reaction.
		 */
		TypeReaction sample(int index, double value);

		Sampler() {/* */};

	};

	template<class TypeReaction>
	Sampler<TypeReaction>::Sampler(const std::map<TypeReaction,std::vector<double>* >& reaction_map) :
	                       nreaction(reaction_map.size()), nenergy(reaction_map.begin()->second->size()),
	                       reactions(nreaction), reaction_matrix(nenergy,nreaction - 1) {

		/* TypeReactions */
		typename std::map<TypeReaction,std::vector<double>* >::const_iterator it_rea = reaction_map.begin();
		int nrea = 0;
		/* Cross sections */
		std::vector<std::vector<double>* > xs_container(nreaction);
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
				total_xs += xs_container[nrea]->at(nerg);

			/* Exclusive scan, to construct the accumulated probability table at this energy */
			double partial_sum = 0;
			for(nrea = 0 ; nrea < nreaction - 1; ++nrea) {
				partial_sum += xs_container[nrea]->at(nerg);
				reaction_matrix(nerg,nrea) = partial_sum / total_xs;
			}
		}

//		std::cout << std::endl << reaction_matrix << std::endl;
//		double value = 0.35;
//		std::cout <<  value << ";" << sample(0,value) << std::endl;
	}

	template<class TypeReaction>
	Sampler<TypeReaction>::Sampler(const std::map<TypeReaction,std::vector<double> >& reaction_map) :
	                       nreaction(reaction_map.size()), nenergy(reaction_map.begin()->second.size()),
	                       reactions(nreaction), reaction_matrix(nenergy,nreaction - 1) {

		/* TypeReactions */
		typename std::map<TypeReaction,std::vector<double> >::const_iterator it_rea = reaction_map.begin();
		int nrea = 0;
		/* Cross sections */
		std::vector<std::vector<double> > xs_container(nreaction);
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
				total_xs += xs_container[nrea][nerg];

			/* Exclusive scan, to construct the accumulated probability table at this energy */
			double partial_sum = 0;
			for(nrea = 0 ; nrea < nreaction - 1; ++nrea) {
				partial_sum += xs_container[nrea][nerg];
				reaction_matrix(nerg,nrea) = partial_sum / total_xs;
			}
		}
	}

	template<class TypeReaction>
	Sampler<TypeReaction>::Sampler(const std::map<TypeReaction,double>& reaction_map) :
	                       nreaction(reaction_map.size()), nenergy(1),
	                       reactions(nreaction), reaction_matrix(nenergy,nreaction - 1) {

		/* TypeReactions */
		typename std::map<TypeReaction,double>::const_iterator it_rea = reaction_map.begin();
		int nrea = 0;
		/* Cross sections */
		std::vector<double> xs_container(nreaction);
		for(; it_rea != reaction_map.end() ; ++it_rea) {
			/* Save the reactions into the reaction container */
			reactions[nrea] = (*it_rea).first;
			/* Save the cross sections */
			xs_container[nrea] = (*it_rea).second;
			/* Count reaction */
			nrea++;
		}

		/* First get the total cross section (of this reactions) at this energy */
		double total_xs = 0.0;
		for(nrea = 0 ; nrea < nreaction ; ++nrea)
			total_xs += xs_container[nrea];

		/* Exclusive scan, to construct the accumulated probability table at this energy */
		double partial_sum = 0;
		for(nrea = 0 ; nrea < nreaction - 1; ++nrea) {
			partial_sum += xs_container[nrea];
			reaction_matrix(0,nrea) = partial_sum / total_xs;
		}

	}

	template<class TypeReaction>
	int Sampler<TypeReaction>::getIndex(const Vector& dat, double val) {
		/* Initial boundaries */
		int lo = 0;
		int hi = nreaction - 2;
		if(val < dat(lo)) return 0;
		if(val > dat(hi)) return nreaction - 1;
		while(hi - lo > 1) {
			/* Check boundaries */
			if ((val > dat(lo)) && (val < dat(lo + 1)))
				return lo + 1;
			else if ((val > dat(hi - 1)) && (val < dat(hi)))
				return hi;
			/* New guess */
			int n = (int)(((double)hi + (double)lo)/2.0);
			/* New boundaries */
			if (val < dat(n)) hi = n;
			else lo = n;
		}
		return lo + 1;
	}

	template<class TypeReaction>
	TypeReaction Sampler<TypeReaction>::sample(int index, double value) {
		if(nreaction == 1) return 0;
		int nrea = getIndex((Vector)reaction_matrix(index,Range::all()),value);
		return reactions[nrea];
	}

} /* namespace Helios */
#endif /* SAMPLER_HPP_ */
