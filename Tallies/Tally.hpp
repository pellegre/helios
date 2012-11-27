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

#ifndef TALLY_HPP_
#define TALLY_HPP_

#include <iostream>
#include <vector>
#include <tbb/spin_mutex.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>

#include "../Common/Common.hpp"

namespace acc = boost::accumulators;

namespace Helios {

/* Accumulator used by the Tally class (mean and standard deviation) */
typedef acc::accumulator_set<double, acc::stats<acc::tag::count, acc::tag::mean, acc::stats<acc::tag::variance> > > Accumulator;

/* Child tally */
class ChildTally {
	double value;
public:
	ChildTally() : value(0.0) {/* */}

	/* Accumulate data */
	void acc(double data) {
		value += data;
	};

	void join(const ChildTally* right) {
		value += right->get();
	};

	/* Return accumulated value */
	double get() const {
		return value;
	};

	/* Clone child */
	ChildTally* clone() const {
		return new ChildTally;
	}

	/* Clear data */
	void clear() {
		value = 0.0;
	}

	~ChildTally() {/* */}
};

/* Base class for tallies */
class Tally {
	/* ID of the tally */
	TallyId user_id;
	/* Prototype of the child accumulator */
	ChildTally* prototype;
	/* Single accumulator */
	Accumulator accum;

	friend std::ostream& operator<<(std::ostream& out, const Tally& q);
public:
	Tally(const TallyId& user_id) : user_id(user_id), prototype(0) {
		/* Create child */
		prototype = new ChildTally;
	}

	/* Return a child tally (to accumulate data on a thread)*/
	ChildTally* getChild() {
		return prototype->clone();
	}

	/* Join child tally and accumulate the data (this should be called on a thread-safe environment) */
	void join(ChildTally* child) {
		/* Join child */
		prototype->join(child);
		/* Clear child */
		child->clear();
	}

	/* Accumulate data using a normalization factor */
	void accumulate(double norm) {
		double value = prototype->get();
		/* Accumulate */
		accum(value / norm);
		/* Clear prototype */
		prototype->clear();
	}

	/* Accumulate data */
	void accumulate() {
		accum(prototype->get());
		/* Clear prototype */
		prototype->clear();
	}

	/* Get mean */
	void print(std::ostream& out) const;

	virtual ~Tally() {
		/* Delete prototype */
		delete prototype;
	}
};

/* Output surface information */
std::ostream& operator<<(std::ostream& out, const Tally& q);

} /* namespace Helios */
#endif /* TALLY_HPP_ */
