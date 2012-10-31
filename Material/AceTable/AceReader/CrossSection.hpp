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

#ifndef CROSSSECTION_HPP_
#define CROSSSECTION_HPP_

#include <vector>
#include <fstream>

namespace Ace {

class CrossSection {

	/* Energy index for the reaction */
	int ie;
	/* Cross section */
	std::vector<double> xs_data;

public:

	CrossSection() : ie(1) {/* default constructor */};
	CrossSection(size_t size) : ie(1), xs_data(size,0.0) {/* default constructor */};
	CrossSection(int ie, const std::vector<double>& xs_data) : ie(ie), xs_data(xs_data) {/* */};
	CrossSection(const CrossSection& xs_copy) : ie(xs_copy.ie), xs_data(xs_copy.xs_data) {/* */};

	/* Dump cross section (ACE format)*/
	void dump(std::ostream& xss) const;
	/* Size on ACE table */
	int getSize() const {return (xs_data.size() + 2);};

	/* Size (real one) */
	size_t size() const {return xs_data.size() + (ie - 1);}

	/* Get cross section data */
	const std::vector<double>& getData() const {return xs_data;}
	/* Get (FORTRAN) index */
	int getIndex() const {return ie;}

	double operator[](int index) const {
		if(index < (ie - 1)) return 0.0;
		else return xs_data[index];
	}

	friend const CrossSection operator+(const CrossSection& left, const CrossSection& right);
};

} /* namespace ACE */
#endif /* CROSSSECTION_HPP_ */
