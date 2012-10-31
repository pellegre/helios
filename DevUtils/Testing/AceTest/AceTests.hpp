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

#ifndef ACETESTS_HPP_
#define ACETESTS_HPP_

#include <string>
#include <algorithm>

#include "../../../Common/Common.hpp"
#include "../../../Material/AceTable/AceReader/Ace.hpp"
#include "../../../Material/AceTable/AceReader/AceUtils.hpp"
#include "../../../Material/AceTable/AceReader/Conf.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

class SimpleAceTest : public ::testing::Test {
protected:
	SimpleAceTest() {/* */}
	virtual ~SimpleAceTest() {/* */}
	void SetUp() {/* */}
	void TearDown() {/* */}

};

TEST_F(SimpleAceTest, SumReactions) {
	using namespace std;
	using namespace Ace;

	/* Library to check */
	std::string library = "c";
	std::string xsdir = Conf::DATAPATH + "/xsdir";

	/* Container of isotopes */
	vector<string> isotopes;
	/* Open XSDIR file*/
	ifstream is(xsdir.c_str());
	string str="";
	if (is.is_open()) {
		while ( is.good() ) {
			getline(is,str);
			if (iStringCompare(str,"directory")) break;
		}
		while ( !is.eof() ) {
			getline(is,str);
			/* Obtain information for construct an ACETable Object */
			if ( str.find(library) != string::npos ) {
				std::istringstream s(str);
				string t;
				s >> t;
				std::remove_if(t.begin(), t.end(), ::isspace);
				isotopes.push_back(t);
			}
		}
	}

	for(vector<string>::const_iterator it = isotopes.begin() ; it != isotopes.end() ; ++it) {
		/* Get table */
		NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable((*it)));

		/* Check cross section MTs calculations */
		CrossSection old_st = ace_table->getTotal();
		CrossSection old_el = ace_table->getElastic();
		CrossSection old_ab = ace_table->getAbsorption();

		/* Get original reactions */
		ReactionContainer old_rea = ace_table->getReactions();

		ace_table->updateBlocks();

		/* Get updated reactions */
		ReactionContainer new_rea = ace_table->getReactions();

		Helios::Log::bok() << " - Checking " << new_rea.name() << Helios::Log::crst <<
				" (awr = " << setw(9) << new_rea.awr() << " , temp = " << setw(9) << new_rea.temp() << ") " << Helios::Log::endl;

		/* Check MAIN cross sections */
		double max_total = checkXS(old_st,ace_table->getTotal());
		double max_ela = checkXS(old_el,ace_table->getElastic());
		double max_abs = checkXS(old_ab,ace_table->getAbsorption());
		double _max_diff = max(max_total,max_abs);
		double max_diff = max(_max_diff,max_ela);
		EXPECT_NEAR(0.0,max_diff,5e8*numeric_limits<double>::epsilon());

		size_t nrea = new_rea.size();

		for(size_t i = 0 ; i < nrea ; i++) {
			double diff = checkXS(old_rea[i].getXS(),new_rea[i].getXS());
			EXPECT_NEAR(0.0,diff,5e8*numeric_limits<double>::epsilon());
		}

		delete ace_table;
	}

}

#endif /* ACETESTS_HPP_ */
