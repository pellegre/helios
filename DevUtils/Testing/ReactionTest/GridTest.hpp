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

	SimpleGridTest() {/* */}
	virtual ~SimpleGridTest() {/* */}
	void SetUp() {
		/* Create grid */
		grid = new Helios::MasterGrid();
	}
	void TearDown() {
		delete grid;
	}

	/* Master grid */
	Helios::MasterGrid* grid;
};

/* Limits */
static double min_value = 01.00e-11;
static double max_value = 20.00e+06;

static inline std::vector<double> initRandom() {
	size_t times = 2000000;
	std::vector<double> values(times);
	for(size_t i = 0 ; i < times ; ++i)
		values[i] = randomNumber(min_value,max_value);
	return values;
}
/* Random values */
std::vector<double> random_values = initRandom();

TEST_F(SimpleGridTest, OrderingValues) {
	/* Push random values into the grid */
	grid->pushGrid(random_values.begin(),random_values.end());
	grid->setup();

	/* Check interpolation */
	size_t checks = 100000;
	for(size_t i = 0 ; i < checks ; ++i) {
		double value = randomNumber(min_value,max_value);
		std::pair<size_t,double> pair_value(0,value);
		grid->interpolate(pair_value);
		if(pair_value.first)
			EXPECT_GE(value,(*grid)[pair_value.first]);
		else
			EXPECT_LE(value,(*grid)[pair_value.first]);

		if(pair_value.first + 1 != grid->size() - 1)
			EXPECT_LE(value,(*grid)[pair_value.first + 1]);
	}
}

class InterpolationGridTest : public ::testing::Test {

protected:

	double linear_function(const double& x) {
		return (a + b * x);
	}

	InterpolationGridTest() {/* */}
	virtual ~InterpolationGridTest() {/* */}

	void SetUp() {
		/* Create grid */
		grid = new Helios::MasterGrid();
		/* Values for the linear function */
		a = 10.00;
		b = 25.00;

		/* Check interpolation */
		size_t user_points = 1000;
		size_t grid_points = 12345 * user_points;

		/* --- User grid */
		user_grid = std::vector<double>(user_points + 1);
		/* Linear values over the grid */
		user_function = std::vector<double>(user_points + 1);
		/* Delta */
		double delta_x = (max_value - min_value) / (double)user_points;
		/* Set the user grid */
		for(size_t i = 0 ; i <= user_points ; ++i) {
			double x = (double) i * delta_x + min_value;
			user_grid[i] = x;
			user_function[i] = linear_function(x);
		}

		/* --- Master grid */
		delta_x = (max_value - min_value) / (double)grid_points;
		std::vector<double> master_grid(grid_points + 1);
		for(size_t i = 0 ; i <= grid_points ; ++i) {
			double x = (double) i * delta_x + min_value;
			master_grid[i] = x;
		}

		/* Setup the master grid */
		grid->pushGrid(master_grid.begin(), master_grid.end());
		grid->setup();

	}

	void TearDown() {
		delete grid;
	}

	/* Master grid */
	Helios::MasterGrid* grid;
	/* Parameters for linear function */
	double a,b;
	/* --- User grid */
	std::vector<double> user_grid;
	/* Linear values over the grid */
	std::vector<double> user_function;
};

TEST_F(InterpolationGridTest, LinearInterpolation) {
	/* Interpolate values on master grid */
	std::vector<double> new_values = grid->interpolate(user_grid,user_function);

	/* Check the interpolated values */
	for(size_t i = 0 ; i < new_values.size() ; ++i) {
		double eval = linear_function((*grid)[i]);
		double inter = new_values[i];
		EXPECT_NEAR(eval,inter,5e10*std::numeric_limits<double>::epsilon());
	}
}

TEST_F(InterpolationGridTest, RandomLinearInterpolation) {
	/* Interpolate values on master grid */
	std::vector<double> new_values = grid->interpolate(user_grid,user_function);

	/* Check the interpolated values */
	for(size_t i = 0 ; i < random_values.size() ; ++i) {
		double eval = linear_function(random_values[i]);
		double factor = 0.0;
		size_t idx = grid->index(random_values[i],factor);
		double inter = factor * (new_values[idx + 1] - new_values[idx]) + new_values[idx];
		EXPECT_NEAR(eval,inter,5e8*std::numeric_limits<double>::epsilon());
	}
}

class ChildGridTest : public ::testing::Test {

protected:

	double linear_function(const double& x) {
		return (a + b * x);
	}

	ChildGridTest() {/* */}
	virtual ~ChildGridTest() {/* */}

	void SetUp() {
		/* Create grid */
		grid = new Helios::MasterGrid();
		/* Values for the linear function */
		a = 10.00;
		b = 25.00;

		/* Number of grids */
		size_t ngrids = 5;
		size_t max_points = 500;
		for(size_t j = 0 ; j < ngrids ; ++j) {
			/* Check interpolation */
			size_t user_points = rand()%max_points + 1;
			/* --- User grid */
			std::vector<double> user_grid = std::vector<double>(user_points + 1);
			/* Linear values over the grid */
			std::vector<double> user_function = std::vector<double>(user_points + 1);
			/* Delta */
			double delta_x = (max_value - min_value) / (double)user_points;
			/* Set the user grid */
			for(size_t i = 0 ; i <= user_points ; ++i) {
				double x = (double) i * delta_x + min_value;
				user_grid[i] = x;
				user_function[i] = linear_function(x);
			}
			user_functions.push_back(user_function);
			/* Push child grid into the master grid */
			child_grids.push_back(grid->pushGrid(user_grid.begin(), user_grid.end()));
		}

		/* Setup master grid */
		grid->setup();
	}

	void TearDown() {
		delete grid;
	}

	/* Master grid */
	Helios::MasterGrid* grid;
	/* Parameters for linear function */
	double a,b;
	/* Linear values over the grid */
	std::vector<std::vector<double> > user_functions;
	/* Child grids */
	std::vector<Helios::ChildGrid*> child_grids;
};

TEST_F(ChildGridTest, LinearInterpolation) {
	for(size_t j = 0 ; j < child_grids.size() ; ++j) {
		Helios::ChildGrid* child_grid = child_grids[j];
		std::vector<double> user_function = user_functions[j];

		/* Check the interpolated values */
		for(size_t i = 0 ; i < random_values.size() ; ++i) {
			double eval = linear_function(random_values[i]);
			double factor = 0.0;
			std::pair<size_t,double> pair_value(0,random_values[i]);
			size_t idx = child_grid->index(pair_value,factor);
			double inter = factor * (user_function[idx + 1] - user_function[idx]) + user_function[idx];
			EXPECT_NEAR(eval,inter,5e8*std::numeric_limits<double>::epsilon());
		}
	}
}

#endif /* GRIDTEST_HPP_ */
