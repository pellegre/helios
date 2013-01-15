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

#include <cstdlib>

#include "DLYBlock.hpp"
#include "../AceUtils.hpp"

using namespace std;
using namespace Ace;

DLYBlock::DLYBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable::AceTable* ace_table)
    : ACEBlock(xss,ace_table),
      DNEDLBlock(nxs,jxs,xss,nxs[NeutronTable::NPCR],ace_table) {

	/* Get the number of MT numbers (i.e. of tabulated cross sections) */
	int nmt = nxs[NeutronTable::NPCR];

	setBegin(xss.begin() + (jxs[NeutronTable::DNU] - 1));

	/* Get NU data */
	int lnu;
	getXSS(lnu);
	nu_data = new NUBlock::Tabular(getPosition(),ace_table);

	/* Location in the table */
	vector<double>::const_iterator it_basic = xss.begin() + ((jxs[NeutronTable::BDD]) - 1);

	for(int i = 0 ; i < nmt ; i++)
		delayed_data.push_back(BasicData(it_basic));

	/* Get the location data */
	vector<int> loca_data = DNEDLBlock.getData();

	for(int i = 0 ; i < nmt ; i++) {
		/* Location in the table */
		int loca = jxs[NeutronTable::DNED] + loca_data[i] - 1;
		/* Set the start point on the XSS table */
		setBegin(xss.begin() + (loca - 1));
		/* Push the angular distribution into the container */
		energy_dist.push_back(EnergyDistribution(getPosition(),xss.begin() + jxs[NeutronTable::DNED] - 1));
	}
}

ReactionContainer& DLYBlock::reas() const {return dynamic_cast<NeutronTable*>(ace_table)->getReactions();};

void DLYBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	nxs[NeutronTable::NPCR] = energy_dist.size();

	int size = 0;
	/* Size of the NU data */
	size += nu_data->getSize() + 1;

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::DNU,size);

	size = 0;

	/* Data for each precursor family */
	for(size_t i = 0 ; i < delayed_data.size() ; i++)
		size += delayed_data[i].getSize();

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::BDD,size);

	DNEDLBlock.updatePointers(nxs,jxs_old,jxs_new);

	size = 0;

	/* Get the size of each XS tabulated */
	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		size += (*it_ener).getSize();

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::DNED,size);
}

/* Dump the block, on a XSS stream */
void DLYBlock::dump(std::ostream& xss) {
	int lnu = 2;
	putXSS(lnu,xss);
	/* NU data */
	nu_data->dump(xss);

	for(size_t i = 0 ; i < delayed_data.size() ; i++)
		delayed_data[i].dump(xss);

	/* Energy distribution */
	DNEDLBlock.dump(xss);
	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		(*it_ener).dump(xss);
}

int DLYBlock::getSize() const {
	int size = nu_data->getSize()+ 1 + DNEDLBlock.getSize();

	vector<BasicData>::const_iterator it_data;
	for(it_data = delayed_data.begin() ; it_data != delayed_data.end() ; it_data++)
		size += (*it_data).getSize();

	/* Get the size of each XS tabulated */
	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		size += (*it_ener).getSize();

	return size;
}

DLYBlock::BasicData::BasicData(std::vector<double>::const_iterator& it) {
	Ace::getXSS(dec,it);
	Ace::getXSS(nr,it);
	Ace::getXSS(nbt,nr,it);
	Ace::getXSS(aint,nr,it);
	Ace::getXSS(ne,it);
	Ace::getXSS(energies,ne,it);
	Ace::getXSS(prob,ne,it);
}

void DLYBlock::BasicData::dump(std::ostream& xss) {
	putXSS(dec,xss);
	putXSS(nr,xss);
	putXSS(nbt,xss);
	putXSS(aint,xss);
	putXSS(ne,xss);
	putXSS(energies,xss);
	putXSS(prob,xss);
}
