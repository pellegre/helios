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

#ifndef MASTERGRID_HPP_
#define MASTERGRID_HPP_

#include <iostream>
#include <vector>

namespace Helios {

	class ChildGrid;

	/*
	 * A class for management floating point grids.
	 *
	 * Usually a MASTER grid represents a set of unionized grids. A MASTER grid
	 * could have CHILD grids each one with its own grid. But each CHILD grid contains
	 * a reference to the PARENT (i.e. MASTER) and a method to map global indexes (pointing
	 * to the MASTER grid) to local indexes (pointing to the CHILD grid).
	 */
	class MasterGrid {
		/* --- Master Grid */
		std::vector<double> master_grid;

		/* --- Coarse Table */
		int size_coarse;
		double delta_coarse;
		std::vector<size_t> coarse_grid;

		/* Container of child grids */
		std::vector<ChildGrid*> child_grids;
	public:
		/* Number of elements to reserve for the grid */
		static size_t reserve_grid;

		MasterGrid();

		/* Size of the grid */
		size_t size() const {return master_grid.size();}

		/*
		 * Push a new energy grid into the master grid and return a child pointer.
		 * Is MASTER responsibility to delete the pointer.
		 */
		template<class InputIterator>
		ChildGrid* pushGrid(InputIterator first, InputIterator last);

		/* Access value on the grid (constant reference because a client can't modify the grid from here) */
		double operator[](size_t index) const {return master_grid[index];}

		/* --- Setup master grid
		 *
		 * This will make the union of all the previously pushed grids (removing
		 * common points)
		 */
		void setup();

		/* --- Interpolation */

		/* Set index on the pair and return interpolation factor */
		double interpolate(std::pair<size_t,double>& pair_value) const;
		/* Given a grid and values on it, returns interpolated values to be used with this grid */
		std::vector<double> interpolate(const std::vector<double>& grid, const std::vector<double>& values) const;
		/* Given just a value, returns the interpolation factor and the index */
		size_t index(const double& value, double& factor) const;
		/* Given a pair of index-value, set the index on the master grid */
		void setIndex(std::pair<size_t,double>& pair_value) const;

		/* Print grid information */
		void print(std::ostream& out) const;

		virtual ~MasterGrid();
	};

	/*
	 * A class for management floating point grids that contains a reference to a MASTER grid
	 *
	 * The access to the data on a CHILD grid is done using MASTER grid indexes.
	 */
	class ChildGrid {

		/* Master Grid */
		const MasterGrid* master_grid;
		/* Child Grid */
		std::vector<double> child_grid;
		/* Master Grid pointers */
		std::vector<size_t> master_pointers;

		/* Private constructor, this can be called ONLY from a Master grid */
		ChildGrid(const MasterGrid* master_grid, const std::vector<double>& child_grid) :
			master_grid(master_grid), child_grid(child_grid) {/* */} ;

		/* Setup child grid (put offsets of MASTER grid) */
		void setup(const std::vector<size_t>& _master_pointers);

		/* Friendly master */
		friend class MasterGrid;
	public:

		/* Size of the CHILD grid */
		size_t size() const {return child_grid.size();}

		/* Access value on the grid (constant reference because a client can't modify the grid from here). */
		double operator[](size_t index) const {return child_grid[index];}

		/* Set MASTER index on the pair, return the index and set interpolation factor on the child grid */
		size_t index(std::pair<size_t,double>& pair_value, double& factor) const;

		/* Print grid information */
		void print(std::ostream& out) const;

		virtual ~ChildGrid() {/* */};
	};

	template<class InputIterator>
	ChildGrid* MasterGrid::pushGrid(InputIterator first, InputIterator last) {
		/* Push data into the common grid */
		master_grid.insert(master_grid.end(), first, last);

		/* Create child grid */
		std::vector<double> child_grid;
		child_grid.reserve(last - first);
		child_grid.insert(child_grid.end(), first, last);

		/* Construct child */
		ChildGrid* child = new ChildGrid(this,child_grid);

		/* Push child into the container */
		child_grids.push_back(child);

		/* Return new child */
		return child;
	}

} /* namespace Helios */
#endif /* MASTERGRID_HPP_ */
