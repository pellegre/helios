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

#ifndef NEUTRONTABLE_HPP_
#define NEUTRONTABLE_HPP_

#include <iostream>
#include <map>
#include "ACETable.hpp"
#include "NeutronReaction.hpp"
#include "ReactionContainer.hpp"

namespace Ace {

class NeutronTable : public AceTable {

	/* Constructor */
	static AceTable* NewTable(const std::string& _table_name, const std::string& full_path, size_t address) {
		return new NeutronTable(_table_name,full_path,address);
	}

	/* General information of the table */
	void printTableInfo(std::ostream& out = std::cout) const;

	void updateData();

	/* Map of MT numbers with the neutron reaction */
	ReactionContainer reactions;

	NeutronTable(const std::string& _table_name, const std::string& full_path, size_t address);

	/*  List of MT numbers and reaction */
	static std::map<int,std::string> mts_reactions;

public:

	/* -------------- Counters location on NXS */

	static const int LGT  = 0;  /* Length of the second block of data */
	static const int ZA   = 1;  /* ZAID */
	static const int NES  = 2;  /* Number of energies */
	static const int NTR  = 3;  /* Number of reactions excluding elastic */
	static const int NR   = 4;  /* Number of reactions having secondary neutrons excluding elastic */
	static const int NTRP = 5;  /* Number of photon production reactions */
	static const int NPCR = 7;  /* Number of delayed neutron precursor families */
	static const int NT   = 14; /* Number of PIKMT reactions */
	static const int PHT  = 15; /* (0) Normal photon production , (-1) do not produce photon */

	/* -------------- Pointers location on JXS */

	static const int ESZ   = 0;  /* Location of energy table */
	static const int NU    = 1;  /* Location of NU data */
	static const int MTR   = 2;  /* Location of MT array */
	static const int LQR   = 3;  /* Location of Q-value array */
	static const int TYR   = 4;  /* Location of reaction type array */
	static const int LSIG  = 5;  /* Location of table of XS locators */
	static const int SIG   = 6;  /* Location of XS */
	static const int LAND  = 7;  /* Location of table of angular distribution locators */
	static const int AND   = 8;  /* Location of angular distribution */
	static const int LDLW  = 9;  /* Location of table of energy distribution locators */
	static const int DLW   = 10; /* Location of energy distribution */
	static const int GPD   = 11; /* Location of photon production data  */
	static const int MTRP  = 12; /* Location of photon production MT array */
	static const int LSIGP = 13; /* Location of photon production XS locators */
	static const int SIGP  = 14; /* Location of photon production XS */
	static const int LANDP = 15; /* Location of photon production angular distribution locators */
	static const int ANDP  = 16; /* Location of photon production angular distributions */
	static const int LDLWP = 17; /* Location of photon production energy distribution locators */
	static const int DLWP  = 18; /* Location of of photon production energy distributions */
	static const int YP    = 19; /* Location of location of table of yield multipliers */
	static const int FIS   = 20; /* Location of total fission cross section */
	static const int END   = 21; /* Last word of the table */
	static const int LUNR  = 22; /* Location of probability tables */
	static const int DNU   = 23; /* Location of delayed nubar data */
	static const int BDD   = 24; /* Location of basic delayed data */
	static const int DNEDL = 25; /* Location of table of energy distribution locators */
	static const int DNED  = 26; /* Location of energy distributions */

	/* Friendly reader */
	friend class AceReader;

	/* Get the reactions containers */
	ReactionContainer& getReactions() {return reactions;};
	const ReactionContainer& getReactions() const {return reactions;};

	/* Get the energy grid a main cross sections */
	const std::vector<double>& getEnergyGrid() const;
	CrossSection getTotal() const;      /* Total cross section */
	CrossSection getAbsorption() const; /* Disappearance cross section */
	CrossSection getElastic() const;    /* Elastic cross section */

	/* Check if the table contains fission information */
	bool isFissile() const {return jxs[NU];}

	/* Get block (throws an exception if the block is not inside the table) */
	template<class Block>
	Block* getBlock() const;

	/* Get a block, return a NULL pointer if the block is not on the table */
	template<class Block>
	Block* block() const;

	virtual ~NeutronTable();

};

template<class Block>
Block* NeutronTable::getBlock() const {
	std::string name = Block::name();
	for(std::vector<ACEBlock*>::const_iterator it = blocks.begin() ; it != blocks.end() ; ++it)
		if(name == (*it)->blockName())
			return dynamic_cast<Block*>((*it));
	throw(AceTableError(this,"Cannot find block " + name));
}

template<class Block>
Block* NeutronTable::block() const {
	std::string name = Block::name();
	for(std::vector<ACEBlock*>::const_iterator it = blocks.begin() ; it != blocks.end() ; ++it)
		if(name == (*it)->blockName())
			return dynamic_cast<Block*>((*it));
	return 0;
}

} /* namespace ACE */

#endif /* NEUTRONTABLE_HPP_ */
