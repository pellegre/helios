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
#include <set>
#include <map>
#include <cmath>
#include <functional>
#include <numeric>

#include "NeutronTable.hpp"
#include "AceUtils.hpp"
#include "PrintMessage.hpp"

#include "../../../Common/Common.hpp"

using namespace std;

namespace Ace {
	size_t shift = 0;
}

bool Ace::iStringCompare( const std::string& str1, const std::string& str2 ) {
    string str1Cpy( str1 );
    string str2Cpy( str2 );
    transform( str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower );
    transform( str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower );
    return ( str1Cpy == str2Cpy );
}

void Ace::shiftJXSArray(const int jxs_old[AceTable::jxs_size], int jxs_new[AceTable::jxs_size], int block_id, int value) {
	int old_ptr = jxs_old[block_id]; /* Before */
	if(!old_ptr) return;

	/* Get all shifts */
	map<int,int> shifts;
	for(int ipos = 0; ipos < AceTable::jxs_size ; ipos++) {
		int ptr = jxs_old[ipos];
		if(ptr && (ipos != 20 ) && (ptr > old_ptr) )
			/*
			 * ipos should be != than 20 because the FIS block is inside the SIG block :-p.
			 * Is not like the FIS block is an independent one, is inside another. Damn pricks.
			 */
			shifts[ptr - old_ptr] = ipos;
	}

	/* The MIN shift should be the old size of the block */
	map<int,int>::iterator it_first = shifts.begin();

	int size = (*it_first).first;

	if((*it_first).second == NeutronTable::END) { /* This is the last block on the table */
		shiftJXSArray(jxs_old,jxs_new,NeutronTable::END,value - size);
		return;
	}

	for(int i = 0; i < AceTable::jxs_size ; i++) {
		if(jxs_old[i] && (jxs_old[i] > old_ptr) ) {
			jxs_new[i] += (value - size);
		}
	}
}

/* Parse ports defined by an interval and push the values into the set */
static void ParseNumbersInt(const string& argv, set<int>* port_values) {
        /* Check if there is an interval in the comma separated value */
        size_t middle = argv.find_first_of("-",0);

        if(middle != string::npos) {

                /* Get both numbers */
                string left = argv.substr(0,middle);
                string right = argv.substr(middle+1);

                /* Convert the string to integers */
                int nleft = atoi(left.c_str());
                int nright = atoi(right.c_str());

                /* Insert each value into the set */
                for(int i = nleft ; i <= nright ; i++)
                        port_values->insert(i);

        }else {

                /* Is just one value */
                int value = atoi(argv.c_str());
                port_values->insert(value);

        }
}

set<int> Ace::getNumbers(const string& argv) {

        /* Set of values */
        set<int> port_values;

        /* Position of comma separated values */
        size_t ini = 0;
        size_t end = argv.find_first_of(",",ini);

        /* Value between commas */
        string port_comma = argv.substr(ini,end-ini);

        ParseNumbersInt(port_comma,&port_values);

        while(end != string::npos) {
                /* Update position */
                ini = end + 1;
                /* Update value between commas */
                end = argv.find_first_of(",",ini);
                port_comma = argv.substr(ini,end-ini);

                ParseNumbersInt(port_comma,&port_values);
        }

        return port_values;
}

double Ace::checkXS(const CrossSection& xs1, const CrossSection& xs2) {
	/* Sanity check, the XS should be referring to the same energy grid */
	if( (xs1.getIndex() != xs2.getIndex()) || (xs1.getData().size() != xs2.getData().size()) )
			 throw(Helios::GeneralError("ACE::checkXS() : Cross sections aren't of the same size. "));

	vector<double> diff(xs1.getData().size());

	for(size_t i = 0 ; i < xs1.getData().size() ; i++) {
		double minus = (xs1.getData()[i] - xs2.getData()[i]);
		diff[i] = minus * minus;
	}

	double norm = inner_product(xs1.getData().begin(),xs1.getData().end(),xs1.getData().begin(),0.0);
	if(norm > 0.0)
		return sqrt(inner_product(diff.begin(),diff.end(),diff.begin(),0.0) / norm);
	return 0.0;
}

vector<double> Ace::getXsDifference(const CrossSection& xs1, const CrossSection& xs2) {
	/* Sanity check, the XS should be referring to the same energy grid */
	if( (xs1.getIndex() != xs2.getIndex()) || (xs1.getData().size() != xs2.getData().size()) )
		 throw(Helios::GeneralError("ACE::getXsDifference() : Cross sections aren't of the same size. "));

	vector<double> diff(xs1.getData().size());

	for(size_t i = 0 ; i < xs1.getData().size() ; i++) {
		double minus = (xs1.getData()[i] - xs2.getData()[i]);
		diff[i] = minus * minus;
	}

	return diff;
}
