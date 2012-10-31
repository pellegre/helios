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

#ifndef NRBLOCK_HPP_
#define NRBLOCK_HPP_

#include <vector>
#include <map>
#include "ACEBlock.hpp"
#include "../NeutronTable.hpp"
#include "../NeutronReaction.hpp"
#include "../CrossSection.hpp"
#include "../AngularDistribution.hpp"
#include "../ReactionContainer.hpp"

namespace Ace {

/*
 * This block manages all the ACE blocks related to neutron reactions (NRBlock = Neutron Reaction Block)
 *
 * MTR Block:  List of ENDF/B MT numbers
 * LQR Block:  List of kinematic Q-values
 * TYR Block:  Information about the type of reaction
 * LSIG Block: List of cross-section locators
 * SIG Block:  Cross sections for all reactions
 * LAND Block: List of angular-distribution locators (include elastic)
 * AND Block:  Angular distributions for all reactions (include elastic)
 * LDLW Block: List of energy distribution locators
 * DLW Block:  Energy distribution for all reactions
 */
class NRBlock: public AceTable::ACEBlock {

	typedef RawBlock<int,NeutronTable::MTR> MTRBlock;
	typedef RawBlock<double,NeutronTable::LQR> LQRBlock;
	typedef RawBlock<int,NeutronTable::TYR> TYRBlock;

	/* Block where the cross section arrays are saved */
	class SIGBlock : public AceTable::ACEBlock {

	private:

		/* Update internal data, in this case the LSIG block */
		void updateData();

		/* Update pointers on the ACE table according  to data on this block */
		void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

		/* Block of locators of cross sections */
		RawBlock<int,NeutronTable::LSIG> LSIGBlock;

		/* Container of XS */
		std::vector<CrossSection> xs;

		SIGBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table);

	public:

		friend class NRBlock; /* Managed by this NRBlock */
		friend class NeutronTable;

		/* Dump the block, on a xss stream */
		void dump(std::ostream& xss);

		int getSize() const;

		int getType() const {return NeutronTable::SIG;};

		static std::string name() {return "SIGBlock";}
		std::string blockName() const {return name();};

		virtual ~SIGBlock() {/* */};
	};

	/* Update internal data of each block */
	void updateData();

	/* Update pointers on the ACE table according  to data on this block */
	void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

	/* Some ACE blocks managed by the block */
	MTRBlock mtr_block;
	LQRBlock lqr_block;
	TYRBlock tyr_block;
	SIGBlock sig_block;

	/* Access to the reaction container on the parent table */
	ReactionContainer& reas() const;

	NRBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table);

public:

	friend class NeutronTable;

	void dump(std::ostream& xss);

	int getSize() const;

	int getType() const;

	static std::string name() {return "NRBlock";}
	std::string blockName() const {return name();};

	virtual ~NRBlock();
};

} /* namespace ACE */
#endif /* NRBLOCK_HPP_ */
