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

#include "Tally.hpp"

using namespace std;

namespace Helios {

void Tally::print(std::ostream& out) const {
	/* Print name */
	out << setw(15) << user_id << " = " << fixed <<
		   setw(9) << acc::mean(accum) << " +- " <<
		   setw(9) << sqrt((double)acc::variance(accum) / (double)acc::count(accum));
}

std::ostream& operator<<(std::ostream& out, const Tally& q) {
	q.print(out);
	return out;
}

/* Get child tallies */
vector<ChildTally*>& TallyContainer::getChildTallies() {
	RequestChildMutex::scoped_lock lock(child_mutex);
	/* If there aren't tallies on the pool, create one */
	if(child_tallies.size() == 0) {
		/* Hopefully this should be done only once for each thread */
		vector<ChildTally*>* new_tallies = new vector<ChildTally*>;
		new_tallies->resize(tallies.size());
		/* Create tallies */
		for(size_t i = 0 ; i < tallies.size() ; ++i)
			(*new_tallies)[i] = tallies[i]->getChild();
		/* Return new container */
		return *new_tallies;
	}
	/* Get container on the back */
	vector<ChildTally*>* tallies_container = child_tallies.back();
	child_tallies.pop_back();
	/* Return reference */
	return *tallies_container;
}

/* Set tallies */
void TallyContainer::setChildTallies(vector<ChildTally*>& tally_container) {
	RequestChildMutex::scoped_lock lock(child_mutex);
	/* Sanity check */
	assert(tally_container.size() == tallies.size());
	/* Push back container */
	child_tallies.push_back(&tally_container);
}

void TallyContainer::reduce() {
	/* Accumulate tallies */
	for(size_t i = 0 ; i < tallies.size() ; ++i) {
		/* Join each tally */
		for(size_t j = 0 ; j < child_tallies.size() ; ++j) {
			/* Join with parent tally */
			tallies[i]->join((*child_tallies[j])[i]);
		}
	}
}

void TallyContainer::accumulate(double norm) {
	/* Accumulate tallies (using initial source weight as a normalization factor) */
	for(size_t i = 0 ; i < tallies.size() ; ++i)
		/* Accumulate each tally */
		tallies[i]->accumulate(norm);
}

void TallyContainer::clear() {
	for(size_t i = 0 ; i < tallies.size() ; ++i)
		/* Accumulate each tally */
		tallies[i]->clear();
}

void TallyContainer::join(TallyContainer& right) {
	for(size_t i = 0 ; i < tallies.size() ; ++i)
		tallies[i]->join(right.tallies[i]);
}

TallyContainer::~TallyContainer() {
	/* Delete tallies */
	purgePointers(tallies);
	/* Delete child tallies */
	for(size_t i = 0 ; i < child_tallies.size() ; ++i) {
		/* Delete each instance */
		purgePointers(*child_tallies[i]);
		delete child_tallies[i];
	}
}

} /* namespace Helios */
