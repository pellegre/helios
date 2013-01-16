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

#ifndef DLYBLOCK_H_
#define DLYBLOCK_H_

#include <iostream>
#include <fstream>
#include <vector>
#include "ACEBlock.hpp"
#include "NUBlock.hpp"
#include "../ACETable.hpp"
#include "../NeutronTable.hpp"
#include "../EnergyDistribution.hpp"

namespace Ace {

/* Virtual block to handle delayed neutron data */
class DLYBlock: public AceTable::ACEBlock {

public:

	class BasicData {
	public:
		double dec;                   /* Decay constant of this group */
		int nr;                       /* Number of interpolation regions */
		std::vector<int> nbt;         /* ENDF interpolation parameters */
		std::vector<int> aint;
		int ne;                       /* Number of energies */
		std::vector<double> energies; /* tabular energies points */
		std::vector<double> prob;     /* Probabilities */

		BasicData() {/* */};
		BasicData(std::vector<double>::const_iterator& it);
		void dump(std::ostream& xss);
		int getSize() const {return (3 + nbt.size() + aint.size() + energies.size() + prob.size());};
		~BasicData() {/* */};
	};

	friend class NeutronTable;

	/* Dump the block, on a xss stream */
	void dump(std::ostream& xss);

	int getSize() const;

	int getType() const {return NeutronTable::DNU;};

	static std::string name() {return "DLYBlock";}
	std::string blockName() const {return name();};

	/* Clone NU data inside this block */
	NUBlock::NuData* clone() const {
		return nu_data->clone();
	}

	/* Get the NU data */
	const NUBlock::NuData& getNuData() const {return *nu_data;};

	const std::vector<BasicData>& getDelayedData() const {
		return delayed_data;
	}

	const std::vector<EnergyDistribution>& getEnergyDistribution() const {
		return energy_dist;
	}

	virtual ~DLYBlock() { delete nu_data; };

private:
	/* Update pointers on the ACE table according  to data on this block */
	void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

	/* NU data of delayed neutrons */
	NUBlock::Tabular* nu_data;
	/* Delayed data for each precusor family */
	std::vector<BasicData> delayed_data;
	/* Block of locators of energy distributions of the delayed data of each precursor groups */
	RawBlock<int,NeutronTable::DNEDL> DNEDLBlock;
	/* Energy distributions of precursor groups */
	std::vector<EnergyDistribution> energy_dist;

	/* Access to the reaction container on the parent table */
	ReactionContainer& reas() const;

	DLYBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable::AceTable* ace_table);
};

} /* namespace ACE */
#endif /* DLYBLOCK_H_ */
