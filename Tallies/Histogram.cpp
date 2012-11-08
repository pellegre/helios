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

#include "Histogram.hpp"
#include "../Common/Common.hpp"

namespace Helios {

using namespace std;

void LinearBins::print(std::ostream& out) const {
	out << "# -- Linear Histogram " << endl;
	out << "# Number of bins : " << (int)values.size() << endl;
	out << "# Limits         : "  << scientific << min << " - " << max << endl;
	out << "# Delta          : "  << scientific << delta << endl;
	/* Print lower - upper and value of each bin */
	out << "#" << setw(14) << "Low" << setw(15) << "High" << setw(15) << "Value" << endl;
	for(size_t i = 0 ; i < values.size() ; ++i)
		out << scientific << setw(15) << (min + i * delta) <<
		setw(15) << (min + (i + 1) * delta) <<
		setw(15) << values[i] << endl;
}

void LinearBins::normalize() {
	for(size_t i = 0 ; i < values.size() ; ++i)
		if(not compareFloating(values[i],0.0))
			values[i] /= total;
}

} /* namespace Helios */
