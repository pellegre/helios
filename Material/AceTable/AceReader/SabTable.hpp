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

#ifndef SABTABLE_HPP_
#define SABTABLE_HPP_

#include <iostream>
#include "ACETable.hpp"

namespace Ace {

class SabTable: public Ace::AceTable {

	/* Constructor */
	static AceTable* NewTable(const std::string& _table_name, const std::string& full_path, size_t address) {
		return new SabTable(_table_name, full_path, address);
	}

	/* General information of the table */
	void printTableInfo(std::ostream& out = std::cout) const;

	void updateData() {
		AceTable::updateData();
	}

	SabTable(const std::string& _table_name, const std::string& full_path, size_t address);

public:

	/* -------------- Counters location on NXS */

	static const int LGT   = 0;  /* Length of the second block of data */
	static const int IDPNI = 1;  /* Inelastic scattering mode */
	static const int NIL   = 2;  /* Inelastic dimensioning parameter */
	static const int NIEB  = 3;  /* Number of inelastic exiting energies */
	static const int IDPNC = 4;  /* Elastic scattering mode */
	static const int NCL   = 5;  /* Elastic dimensioning parameter */
	static const int IFENG = 6;  /* Secondary energy mode */

	/* -------------- Pointers location on JXS */

	static const int ITIE   = 0;  /* Location of inelastic energy table */
	static const int ITIX   = 1;  /* Location of inelastic cross section */
	static const int ITXE   = 2;  /* Location of inelastic energy energy / angle distributions */
	static const int ITCE   = 3;  /* Location of elastic energy table */
	static const int ITCX   = 4;  /* Location of elastic cross section */
	static const int ITCA   = 5;  /* Location of elastic angular distributions */

	/* Friendly reader */
	friend class AceReader;

	/* Get block (throws an exception if the block is not inside the table) */
	template<class Block>
	Block* getBlock() const;

	/* Get a block, return a NULL pointer if the block is not on the table */
	template<class Block>
	Block* block() const;

	virtual ~SabTable();

};

template<class Block>
Block* SabTable::getBlock() const {
	std::string name = Block::name();
	for(std::vector<ACEBlock*>::const_iterator it = blocks.begin() ; it != blocks.end() ; ++it)
		if(name == (*it)->blockName())
			return dynamic_cast<Block*>((*it));
	throw(AceTableError(this,"Cannot find block " + name));
}

template<class Block>
Block* SabTable::block() const {
	std::string name = Block::name();
	for(std::vector<ACEBlock*>::const_iterator it = blocks.begin() ; it != blocks.end() ; ++it)
		if(name == (*it)->blockName())
			return dynamic_cast<Block*>((*it));
	return 0;
}

} /* namespace Ace */
#endif /* SABTABLE_HPP_ */
