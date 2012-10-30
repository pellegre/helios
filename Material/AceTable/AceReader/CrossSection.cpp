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

#include <algorithm>

#include "CrossSection.hpp"
#include "AceUtils.hpp"
#include "PrintMessage.hpp"

using namespace std;
using namespace ACE;

void CrossSection::dump(std::ostream& xss) const {
	putXSS(ie,xss);
	int size = xs_data.size();
	putXSS(size,xss);
	putXSS(xs_data,xss);
}

namespace ACE {

const CrossSection operator+(const CrossSection& left, const CrossSection& right) {
	/* Check for "NULL" cross section */
	if((left.ie + left.xs_data.size()) == 1)
			return right;
	if((right.ie + right.xs_data.size()) == 1)
			return left;

	/* Sanity check, the XS should be referring to the same energy grid */
	if( (left.ie + left.xs_data.size()) != (right.ie + right.xs_data.size()) ) {
		printMessage(PrintCodes::PrintError,"CrossSection::operator+()",
					 "Cross sections aren't of the same size. Aborting. ");

		exit(1);
	}

	/* Get the minor index on both cross sections */
	int min_ie = min(left.ie,right.ie);

	/* Get the max size of the cross section array */
	size_t max_size = max(left.xs_data.size(),right.xs_data.size());

	/* Create new XS */
	vector<double> xs_data(max_size,0.0);

	int ie = min_ie;
	int cnt_left = 0;
	int cnt_right = 0;
	/* Sum the data on each table */
	for(size_t i = 0 ; i < max_size ; i++) {
		if(ie >= left.ie) {
			xs_data[i] += left.xs_data[cnt_left];
			cnt_left++;
		}
		if(ie >= right.ie) {
			xs_data[i] += right.xs_data[cnt_right];
			cnt_right++;
		}
		ie++;
	}

	return CrossSection(min_ie,xs_data);
}

}
