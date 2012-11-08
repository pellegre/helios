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

#ifndef HISTOGRAM_HPP_
#define HISTOGRAM_HPP_

#include <iostream>
#include <vector>

namespace Helios {

class LinearBins {
	/* Accumulated values */
	std::vector<double> values;
	/* Limits on the histogram */
	double min, max, delta;
	/* Total counter */
	double total;
public:

	LinearBins(double min, double max, size_t nbins) :
		values(nbins, 0.0), min(min), max(max), delta((max - min) / (double)nbins), total(0.0) {/* */}

	/* Accumulate value */
	void accum(double value) {
		/* Check limits */
		if(value > min && value < max) {
			/* Accumulate value on histogram bin */
			double diff = (value - min);
			/* Calculate bin position */
			size_t pos = diff / delta;
			/* Accumulate */
			values[pos]++;
			/* Increment counter */
			total++;
		}
	}

	/* Normalize bins */
	void normalize();

	/* Print bins */
	void print(std::ostream& out) const;

	~LinearBins() {/* */}
};

template<class Accumulator>
class Histogram : public Accumulator {
public:
	/* Print operator */
	template<class accum>
	friend std::ostream& operator<<(std::ostream& out, const Histogram<accum>& q);

	/* Constructor (linear) */
	Histogram(double min, double max, size_t nbins) : Accumulator(min, max, nbins) {/* */};

	/* Accumulate value */
	void operator()(double value) {
		Accumulator::accum(value);
	}

	void print(std::ostream& out) const {
		Accumulator::print(out);
	}

	~Histogram() {/* */};
};

template<class accum>
std::ostream& operator<<(std::ostream& out, const Histogram<accum>& q) {
	q.print(out);
	return out;
}

} /* namespace Helios */
#endif /* HISTOGRAM_HPP_ */
