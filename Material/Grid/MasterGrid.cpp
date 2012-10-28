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
#include <algorithm>
#include <cassert>

#include "MasterGrid.hpp"
#include "../../Common/Common.hpp"

using namespace std;

namespace Helios {

/* By default the coarse mesh is half the master grid */
size_t MasterGrid::coarse_grid_factor = 2;
/* By default, 10000 points are reserved for the grid */
size_t MasterGrid::reserve_grid = 10000;

MasterGrid::MasterGrid() {
	/* Reserve space for the grids */
	master_grid.reserve(reserve_grid);
};

void MasterGrid::setup() {
	/* --- Setup MASTER grid */
	sort(master_grid.begin(), master_grid.end());
	vector<double>::const_iterator it_master = unique(master_grid.begin(), master_grid.end());
	master_grid.resize(it_master - master_grid.begin());
}

double MasterGrid::interpolate(pair<size_t,double>& pair_value) const {
	/* Maximum and minimum values for energy */
	double min_energy = master_grid[0];
	double max_energy = master_grid[master_grid.size() - 1];

	/* Energy value */
	double energy = pair_value.second;

	/* First check if the given energy is out of bound */
	if(energy <= min_energy) {
		pair_value.first = 0;
		return 0.0;
	} else if(energy >= max_energy) {
		pair_value.first = master_grid.size() - 2;
		return 1.0;
	}

	/* Bug catcher, in normal conditions this shouldn't happen */
	assert(pair_value.first <= master_grid.size() - 2);

	/* Energy bounds */
	double low_energy = master_grid[pair_value.first];
	double high_energy = master_grid[pair_value.first + 1];
	/* Interpolation factor */
	double factor = (energy - low_energy) / (high_energy - low_energy);

	/* Check if the index is in the right place */
	if(energy >= low_energy && energy < high_energy) {
		/* Don't touch the index and return the factor */
		return factor;

	} else {

		/* Search boundaries */
		vector<double>::const_iterator begin = master_grid.begin();
		vector<double>::const_iterator end = master_grid.end();

		/* Update index */
		pair_value.first = upper_bound(begin, end, energy) - master_grid.begin() - 1;

		/* Return factor */
		return factor;
	}
}

double MasterGrid::interpolate(const double& value) const {
	pair<size_t,double> pair_value(0,value);
	return interpolate(pair_value);
}

vector<double> MasterGrid::interpolate(vector<double>& grid, vector<double>& values) const {
	/* Sanity check */
	assert(grid.size() == values.size());
	/* New container for interpolated values */
	vector<double> new_values(master_grid.size());

	/* We can use a Master grid to setup this new values */
	MasterGrid temp_grid;
	/* Push values into the grid */
	temp_grid.pushGrid(grid.begin(), grid.end());
	/* Setup the grid */
	temp_grid.setup();

	/* For each value on this master grid, we interpolate in the temporary grid */
	pair<size_t,double> pair_value(0,0.0);
	for(size_t i = 0 ; i < master_grid.size() ; ++i) {
		/* Set as a value the one on this master grid */
		pair_value.second = master_grid[i];
		/* Get interpolation factor */
		double factor = temp_grid.interpolate(pair_value);
		/* Set new value */
		new_values[i] = factor * values[pair_value.first];
	}

	return new_values;
}

void MasterGrid::print(ostream& out) const {
	out << Log::ident(1) << "Master grid" << endl;
	out << Log::ident(2) << " - Size of the master grid : " << master_grid.size() << endl;
	out << Log::ident(1) << "Master grid : " << scientific << endl;
	copy(master_grid.begin(), master_grid.end(), ostream_iterator<double>(out," , "));
}

} /* namespace Helios */
