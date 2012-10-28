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

#ifndef GRIDTEST_HPP_
#define GRIDTEST_HPP_

#include <string>

#include "../../../Common/Common.hpp"
#include "../../../Material/Grid/MasterGrid.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

class SimpleGridTest : public ::testing::Test {
protected:
	static std::vector<double> random_values;

	SimpleGridTest() {/* */}
	virtual ~SimpleGridTest() {/* */}
	void SetUp() {/* */}
	void TearDown() {/* */}
	Helios::MasterGrid grid;
};

static inline std::vector<double> initRandom() {
	size_t times = 20000000;
	std::vector<double> values(times);
	for(size_t i = 0 ; i < times ; ++i)
		values[i] = randomNumber(1.00e-11,20.00e+06);
	return values;
}
std::vector<double> SimpleGridTest::random_values = initRandom();

TEST_F(SimpleGridTest, OrderingValues) {
	grid.pushGrid(random_values.begin(),random_values.end());
	grid.setup();

	size_t checks = 2000000;
	for(size_t i = 0 ; i < checks ; ++i) {
		double value = randomNumber(1.00e-11,20.00e+06);
		std::pair<size_t,double> pair_value(0,value);
		grid.interpolate(pair_value);
		if(pair_value.first)
			EXPECT_GE(value,grid[pair_value.first]);
		else
			EXPECT_LE(value,grid[pair_value.first]);
		if(pair_value.first + 1 != grid.size() - 1)
			EXPECT_LE(value,grid[pair_value.first + 1]);
	}
}


#endif /* GRIDTEST_HPP_ */
