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
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>

namespace acc = boost::accumulators;

namespace Helios {

/* Accumulator used by the Tally class (mean and standard deviation) */
typedef acc::accumulator_set<double, acc::stats<acc::tag::mean, acc::tag::moment<2> > > Accumulator;

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
	/* Single accumulator */
	Accumulator accum;
	/* Prototype of the child accumulator */
	ChildTally* child_tally;
public:
	Tally() : child_tally(0) {
		/* Create child */
		child_tally = new ChildTally;
	}

	/* Return a child tally (to accumulate data on a thread)*/
	ChildTally* getChild() const {
		return child_tally->clone();
	}

	/* Join child tally and accumulate the data (this should be called on a thread-safe environment) */
	void join(const ChildTally* child) {
		/* Join child */
		child_tally->join(child);
	}

	/* Accumulate data using a normalization factor */
	void accumulate(double norm) {
		double value = child_tally->get();
		/* Accumulate */
		accum(value / norm);
		/* Clear prototype */
		child_tally->clear();
	}

	/* Accumulate data */
	void accumulate() {
		accum(child_tally->get());
		/* Clear prototype */
		child_tally->clear();
	}

	/* Get mean */
	double average() const {
		return acc::mean(accum);
	}

	/* Get standard deviation */
	double std() const {
		return acc::moment<2>(accum);
	}

	virtual ~Tally() {
		/* Delete child */
		delete child_tally;
	}
};

} /* namespace Helios */
#endif /* TALLY_HPP_ */
