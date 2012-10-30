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

#ifndef DLWBLOCK_HPP_
#define DLWBLOCK_HPP_

#include <vector>
#include <map>
#include <fstream>
#include "ACEBlock.hpp"
#include "../ACETable.hpp"
#include "../NeutronTable.hpp"
#include "../EnergyDistribution.hpp"
#include "../NRContainer.hpp"

namespace ACE {

class DLWBlock: public ACE::ACETable::ACEBlock {

	/* Update internal data, in this case the LDLW block */
	void updateData();

	/* Update pointers on the ACE table according  to data on this block */
	void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

	/* Block of locators of cross sections */
	RawBlock<int,NeutronTable::LDLW> LDLWBlock;

	/* Energy distributions */
	std::vector<EnergyDistribution> energy_dist;

	/* Neutron released distribution, if any */
	std::map<int,TyrDistribution> tyr_dist;

	/* Access to the reaction container on the parent table */
	NRContainer& reas() const;

	DLWBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss,
			ACETable* ace_table, const std::vector<int>& tyrs, const std::vector<int>& mats);

public:

	friend class NeutronTable;

	/* Dump the block, on a xss stream */
	void dump(std::ostream& xss);

	int getSize() const;

	int getType() const {return NeutronTable::DLW;};

	virtual ~DLWBlock() {/* */};

};

} /* namespace ACE */
#endif /* DLWBLOCK_HPP_ */
