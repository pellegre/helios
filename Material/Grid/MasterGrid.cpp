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

/* By default, 10000 points are reserved for the grid */
size_t MasterGrid::reserve_grid = 10000;

MasterGrid::MasterGrid(){
	/* Reserve space for the grids */
	master_grid.reserve(reserve_grid);
};

void MasterGrid::setup() {
	/* Setup MASTER grid */
	sort(master_grid.begin(), master_grid.end());
	vector<double>::const_iterator it_master = unique(master_grid.begin(), master_grid.end());
	master_grid.resize(it_master - master_grid.begin());

	/* Setup child grids */
	for(vector<ChildGrid*>::const_iterator it = child_grids.begin() ; it != child_grids.end() ; ++it) {
		/* Create master array of pointers */
		vector<size_t> master_pointers(size());

		/* Energy limits on child grid */
		double min_energy = (*it)->child_grid[0];
		double max_energy = (*it)->child_grid[(*it)->size() - 1];

		for(size_t i = 0 ; i < size() ; ++i) {
			/* Energy value (on the master grid) */
			double energy = master_grid[i];
			/* First check if the given energy is out of bound */
			if(energy <= min_energy)
				/* Set the pointer to the beginning of the the grid */
				master_pointers[i] = 0;
			else if(energy >= max_energy)
				/* Set the pointer to the end of the grid */
				master_pointers[i] = (*it)->size() - 2;
			else
				/* Get the index on the child grid */
				master_pointers[i] = upper_bound((*it)->child_grid.begin(), (*it)->child_grid.end(), energy) - (*it)->child_grid.begin() - 1;
		}

		/* Setup master pointer */
		(*it)->setup(master_pointers);
	}

	/* Setup coarse grid */
	size_coarse = 20 * master_grid.size();
	coarse_grid.resize(size_coarse);

	/* Get bound energies */
	double emin = master_grid[0];
	double emax = master_grid[master_grid.size() - 1];
	delta_coarse = log(emax / emin)/(size_coarse - 1);

	/* Calculate coarse grid */
	size_t i = 0;
	for (double erg = emin ; erg < emax; erg *= exp(delta_coarse)) {
		coarse_grid[i] = upper_bound(master_grid.begin(), master_grid.end(), erg) - master_grid.begin() - 1;
		++i;
	}
	/* Map the last index */
	coarse_grid[size_coarse-1] = master_grid.size() - 1;
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

	/* Check if the index is in the right place */
	if(energy >= low_energy && energy <= high_energy) {
		/* Don't touch the index and return the factor */
		return (energy - low_energy) / (high_energy - low_energy);
	} else {
		/* Get coarse index */
		size_t coarse_index = (size_t) (log(energy/min_energy) / delta_coarse);

		/* Search boundaries */
		vector<double>::const_iterator begin = master_grid.begin() + coarse_grid[coarse_index];
		vector<double>::const_iterator end = master_grid.begin() + coarse_grid[coarse_index + 1] + 1;

		/* Update index */
		pair_value.first = upper_bound(begin, end, energy) - master_grid.begin() - 1;

		/* Energy bounds */
		double low = master_grid[pair_value.first];
		double high = master_grid[pair_value.first + 1];

		/* Return factor */
		return (energy - low) / (high - low);
	}
}

void MasterGrid::setIndex(std::pair<size_t,double>& pair_value) const {
	/* Maximum and minimum values for energy */
	double min_energy = master_grid[0];
	double max_energy = master_grid[master_grid.size() - 1];

	/* Energy value */
	double energy = pair_value.second;

	/* First check if the given energy is out of bound */
	if(energy <= min_energy) {
		pair_value.first = 0;
		return;
	} else if(energy >= max_energy) {
		pair_value.first = master_grid.size() - 2;
		return;
	}

	/* Bug catcher, in normal conditions this shouldn't happen */
	assert(pair_value.first <= master_grid.size() - 2);

	/* Energy bounds */
	double low_energy = master_grid[pair_value.first];
	double high_energy = master_grid[pair_value.first + 1];

	/* Check if the index is in the right place */
	if(not (energy >= low_energy && energy <= high_energy)) {
		/* Get coarse index */
		size_t coarse_index = (size_t) (log(energy/min_energy) / delta_coarse);

		/* Search boundaries */
		vector<double>::const_iterator begin = master_grid.begin() + coarse_grid[coarse_index];
		vector<double>::const_iterator end = master_grid.begin() + coarse_grid[coarse_index + 1] + 1;

		/* Update index */
		pair_value.first = upper_bound(begin, end, energy) - master_grid.begin() - 1;
	}
}

size_t MasterGrid::index(const double& value, double& factor) const {
	pair<size_t,double> pair_value(0,value);
	factor = interpolate(pair_value);
	return pair_value.first;
}

vector<double> MasterGrid::interpolate(const vector<double>& grid, const vector<double>& values) const {
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
		new_values[i] = factor * (values[pair_value.first + 1] - values[pair_value.first]) + values[pair_value.first];
	}

	return new_values;
}

void MasterGrid::print(ostream& out) const {
	out << Log::ident(1) << "Master grid" << endl;
	out << Log::ident(2) << " - Size of the master grid : " << master_grid.size() << endl;
	out << Log::ident(1) << "Master grid : " << scientific << endl;
	copy(master_grid.begin(), master_grid.end(), ostream_iterator<double>(out," , "));
}

MasterGrid::~MasterGrid() {
	for(vector<ChildGrid*>::const_iterator it = child_grids.begin() ; it != child_grids.end() ; ++it)
		delete (*it);
};

void ChildGrid::setup(const std::vector<size_t>& _master_pointers) {
	master_pointers.reserve(_master_pointers.size());
	/* Copy data */
	master_pointers.insert(master_pointers.end(), _master_pointers.begin(), _master_pointers.end());
}

size_t ChildGrid::index(std::pair<size_t,double>& pair_value, double& factor) const {
	/* Maximum and minimum values for energy */
	double min_energy = child_grid[0];
	double max_energy = child_grid[child_grid.size() - 1];

	/* Energy value */
	double energy = pair_value.second;

	/* First check if the given energy is out of bound */
	if(energy <= min_energy) {
		factor = 0.0;
		return 0;
	} else if(energy >= max_energy) {
		factor = 1.0;
		return child_grid.size() - 2;
	}

	/* Get index from master grid */
	master_grid->setIndex(pair_value);
	size_t child_index = master_pointers[pair_value.first];
	/* Energy bounds */
	double low_energy = child_grid[child_index];
	double high_energy = child_grid[child_index + 1];
	/* Set the interpolation factor */
	factor = (pair_value.second - low_energy) / (high_energy - low_energy);
	return child_index;
}

void ChildGrid::print(std::ostream& out) const {
	out << Log::ident(1) << "Child grid" << endl;
	out << Log::ident(2) << " - Size of the child grid : " << child_grid.size() << endl;
	out << Log::ident(1) << "Child grid : " << scientific << endl;
	copy(child_grid.begin(), child_grid.end(), ostream_iterator<double>(out," , "));
	out << endl;
	out << Log::ident(1) << "Master pointers : " << dec << endl;
	copy(master_pointers.begin(), master_pointers.end(), ostream_iterator<size_t>(out," , "));
}

} /* namespace Helios */
