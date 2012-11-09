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

#include "NRBlock.hpp"
#include "../NeutronTable.hpp"
#include "../AceUtils.hpp"

using namespace std;
using namespace Ace;


NRBlock::NRBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table)
    : ACEBlock(xss,ace_table) ,
      mtr_block(nxs,jxs,xss,nxs[NeutronTable::NTR],ace_table),
      lqr_block(nxs,jxs,xss,nxs[NeutronTable::NTR],ace_table),
      tyr_block(nxs,jxs,xss,nxs[NeutronTable::NTR],ace_table),
      sig_block(nxs,jxs,xss,ace_table) {/* */}

void NRBlock::dump(std::ostream& xss) {
	mtr_block.dump(xss);
	lqr_block.dump(xss);
	tyr_block.dump(xss);
	sig_block.dump(xss);
};

int NRBlock::getSize() const {
	int size = 0;

	size += mtr_block.getSize();
	size += lqr_block.getSize();
	size += tyr_block.getSize();
	size += sig_block.getSize();

	return size;
};

ReactionContainer& NRBlock::reas() const {return dynamic_cast<NeutronTable*>(ace_table)->getReactions();};

/* Update pointers on the ACE table according  to data on this block */
void NRBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	/* And we should set the NXS value */
	nxs[NeutronTable::NTR] = reas().size() - 1;
	mtr_block.updatePointers(nxs,jxs_old,jxs_new);
	lqr_block.updatePointers(nxs,jxs_old,jxs_new);
	tyr_block.updatePointers(nxs,jxs_old,jxs_new);
	sig_block.updatePointers(nxs,jxs_old,jxs_new);
}

void NRBlock::updateData() {
	/* We should update the data with the Reactions container */
	int nrea = reas().size() - 1; /* (the first one is the elastic reaction) */

	ReactionContainer::iterator it_rea;

	/* Prepare container for reactions information */
	vector<int> new_mtr(nrea);
	vector<double> new_lqr(nrea);
	vector<int> new_tyr(nrea);
	vector<CrossSection> new_xs(nrea);

	int cntrea = 0;
	for(it_rea = reas().begin() + 1; it_rea != reas().end() ; it_rea++) {
		/* Get this neutron reaction */
		NeutronReaction rea = (*it_rea);
		new_mtr[cntrea] = rea.getMt();
		new_lqr[cntrea] = rea.getQ();
		new_tyr[cntrea] = rea.getTyr().getTyr();
		new_xs[cntrea] = rea.getXs();
		cntrea++;
	}

	/* Update the data of each block */
	mtr_block.setData(new_mtr);
	lqr_block.setData(new_lqr);
	tyr_block.setData(new_tyr);
	sig_block.xs = new_xs;

	mtr_block.updateData();
	lqr_block.updateData();
	tyr_block.updateData();
	sig_block.updateData();
}

int NRBlock::getType() const {
	return 0;
};

NRBlock::~NRBlock() { /* */ }

NRBlock::SIGBlock::SIGBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table)
    : ACEBlock(xss,ace_table), LSIGBlock(nxs,jxs,xss,nxs[NeutronTable::NTR],ace_table) {

	/* Get the number of MT numbers (i.e. of tabulated cross sections) */
	int nmt = nxs[NeutronTable::NTR];

	/* Get the location data */
	vector<int> lsig_data = LSIGBlock.getData();
	xs.resize(nmt);
	for(int i = 0 ; i < nmt ; i++) {
		/* Location in the table */
		int loca = jxs[NeutronTable::SIG] + lsig_data[i] - 1;
		/* Set the start point on the XSS table */
		setBegin(xss.begin() + (loca - 1));
		/* Number of energies and cross section data */
		int ne;
		int index;
		vector<double> xs_data;
		/* Get cross section values */
		getXSS(index);
		getXSS(ne);
		getXSS(xs_data,ne);
		xs[i] = CrossSection(index,xs_data);
	}

}

void NRBlock::SIGBlock::updateData() {
	/* New array to hold the location of the tabulated XS */
	vector<int> new_lsig;
	/* Current number of cross sections */
	int xs_size = xs.size();
	new_lsig.resize(xs_size);

	/* Initial position (first xs is at "offset" 1) */
	new_lsig[0] = 1;
	for(int i = 1 ; i < xs_size ; i++)
		new_lsig[i] = new_lsig[i - 1] + xs[i - 1].getSize();

	/* Set the new values of the XS location */
	LSIGBlock.setData(new_lsig);
}

/* Update pointers on the ACE table according  to data on this block */
void NRBlock::SIGBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	LSIGBlock.updatePointers(nxs,jxs_old,jxs_new);

	int size = 0;
	/* Get the size of each XS tabulated */
	vector<CrossSection>::const_iterator it;
	for(it = xs.begin() ; it != xs.end() ; it++)
		size += (*it).getSize();

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::SIG,size);
}

/* Dump the block, on a XSS stream */
void NRBlock::SIGBlock::dump(std::ostream& xss) {
	LSIGBlock.dump(xss);
	vector<CrossSection>::iterator it;
	for(it = xs.begin() ; it != xs.end() ; it++)
		(*it).dump(xss);
}

int NRBlock::SIGBlock::getSize() const {
	/* LSIG block size */
	int size = LSIGBlock.getSize();

	/* Get the size of each XS tabulated */
	vector<CrossSection>::const_iterator it;
	for(it = xs.begin() ; it != xs.end() ; it++)
		size += (*it).getSize();

	return size;
}
