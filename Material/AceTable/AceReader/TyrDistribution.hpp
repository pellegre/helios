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
 DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TYRDISTRIBUTION_HPP_
#define TYRDISTRIBUTION_HPP_

#include <vector>
#include <iostream>
#include <fstream>

namespace ACE {

class TyrDistribution {
	/* Tabular data form of NU array */
	int nr;                       /* Number of interpolation regions */
	std::vector<int> nbt;         /* ENDF interpolation parameters */
	std::vector<int> aint;
	int ne;                       /* Number of energies */
	std::vector<double> energies; /* tabular energies points */
	std::vector<double> nu;       /* Values of NU */

	/* Just a number (without energy distribution) */
	int tyr;

	/* Type of data */
	int type;

public:
	/* Type of data */
	static const int distribution = 0;
	static const int number = 1;

	TyrDistribution(int tyr,std::vector<double>::const_iterator it);
	TyrDistribution(int tyr) : tyr(tyr), type(number) {/* */};
	TyrDistribution(const TyrDistribution& tyrr);
	TyrDistribution() : tyr(0), type(number) {/* */};

	void dump(std::ostream& xss) const;
	int getSize() const {return (2 + nbt.size() + aint.size() + energies.size() + nu.size());};
	int getType() const {return type;};
	int getTYR() const {return tyr;};
	~TyrDistribution() {/* */};
};

} /* namespace ACE */
#endif /* TYRDISTRIBUTION_HPP_ */
