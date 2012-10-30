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

#ifndef ANDBLOCK_HPP_
#define ANDBLOCK_HPP_

#include <vector>
#include <fstream>
#include "ACEBlock.hpp"
#include "../ACETable.hpp"
#include "../NeutronTable.hpp"
#include "../AngularDistribution.hpp"
#include "../NRContainer.hpp"

namespace ACE {

/* Block where the angular distribution of the cross section arrays are saved */
class ANDBlock : public ACETable::ACEBlock {

private:

	/* Update internal data, in this case the LAND block */
	void updateData();

	/* Update pointers on the ACE table according  to data on this block */
	void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

	/* Block of locators of cross sections */
	RawBlock<int,NeutronTable::LAND> LANDBlock;

	std::vector<AngularDistribution> and_dist;

	/* Access to the reaction container on the parent table */
	NRContainer& reas() const;

	ANDBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, ACETable* ace_table);

public:

	friend class NeutronTable;

	/* Dump the block, on a xss stream */
	void dump(std::ostream& xss);

	int getSize() const;

	int getType() const {return NeutronTable::AND;};

	static std::string name() {return "ANDBlock";}
	std::string blockName() const {return name();};

	virtual ~ANDBlock() {/* */};
};

} /* namespace ACE */
#endif /* ANDBLOCK_HPP_ */
