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

#ifndef ACEUTILS_HPP_
#define ACEUTILS_HPP_

#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <set>
#include <sstream>

#include "ACETable.hpp"
#include "NeutronReaction.hpp"
#include "CrossSection.hpp"

namespace Ace {

	template<typename T>
	T fromString(const std::string& s) {
		std::istringstream is(s);
		T t;
		is >> t;
		return t;
	}

	template<typename T>
		std::string toString(const T& t) {
		std::ostringstream s;
		s << t;
		return s.str();
	}

	/* Case insensitive string comparison */
	bool iStringCompare( const std::string& str1, const std::string& str2 );

	extern size_t shift;
	/* Put data on XSS table, could be a double or integer */
	inline void putXSS(double xss, std::ostream& out) {
		out.width(20);
		out << std::scientific << std::setprecision(11) << xss;
		shift++;
		if(shift%4 == 0) out << std::endl;
	}
	inline void putXSS(int xss, std::ostream& out) {
		out.width(20);
		out << xss;
		shift++;
		if(shift%4 == 0) out << std::endl;
	}
	template<class Seq>
	void putXSS(const Seq& v, std::ostream& out) {
		size_t table_length = v.size();
		for(size_t i = 0 ; i < table_length ; i++) {
			putXSS(v[i],out);
		}
	}

	template<class T, class FowardIterator>
	void getXSS(T& number, FowardIterator& it) {
		number = T((*it));
		++it;
	}
	template<class Seq, class FowardIterator>
	void getXSS(Seq& v, size_t table_length, FowardIterator& it) {
		v.resize(table_length);
		for(size_t i = 0 ; i < v.size() ; i++)
			getXSS(v[i],it);
	}

	/* Generic JXS shift function */
	void shiftJXSArray(const int jxs_old[AceTable::jxs_size], int jxs_new[AceTable::jxs_size], int block_id, int value);

	/* Parse a string of a range of numbers */
	std::set<int> getNumbers(const std::string& argv);

	/* Check the MAX difference on a value of two XS */
	double checkXS(const CrossSection& xs1, const CrossSection& xs2);
	std::vector<double> getXsDifference(const CrossSection& xs1, const CrossSection& xs2);

}


#endif /* ACEUTILS_HPP_ */
