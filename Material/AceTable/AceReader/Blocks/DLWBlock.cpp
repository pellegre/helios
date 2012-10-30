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

#include "DLWBlock.hpp"
#include "../Utils.hpp"

using namespace std;
using namespace ACE;

DLWBlock::DLWBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss,
		           ACETable* ace_table, const vector<int>& tyrs, const vector<int>& mats)
    : ACEBlock(xss,ace_table), LDLWBlock(nxs,jxs,xss,nxs[NeutronTable::NR],ace_table) {

	/* Get the number of MT numbers (i.e. of tabulated cross sections) */
	int nmt = nxs[NeutronTable::NR];

	/* Get the location data */
	vector<int> land_data = LDLWBlock.getData();

	for(int i = 0 ; i < nmt ; i++) {
		/* Location in the table */
		int loca = jxs[NeutronTable::DLW] + land_data[i] - 1;
		/* Set the start point on the XSS table */
		setBegin(xss.begin() + (loca - 1));
		/* Push the angular distribution into the container */
		energy_dist.push_back(EnergyDistribution(getPosition(),xss.begin() + jxs[NeutronTable::DLW] - 1));

		if(abs(tyrs[i]) > 100) {
			/* Insert the data of the TYR distribution */
			tyr_dist[mats[i]] = TyrDistribution(tyrs[i],xss.begin() + jxs[NeutronTable::DLW] + abs(tyrs[i]) - 102);
		}
	}
}

NRContainer& DLWBlock::reas() const {return dynamic_cast<NeutronTable*>(ace_table)->getReactions();};

void DLWBlock::updateData() {
	energy_dist.clear();
	tyr_dist.clear();

	NRContainer::iterator it_rea;

	for(it_rea = reas().begin() + 1; it_rea != reas().end() ; it_rea++) {
		/* The first reaction with no data for the angular distribution defines the end of the NR array */
		if((*it_rea).getDLW().getKind() == EnergyDistribution::no_data) break;
		energy_dist.push_back((*it_rea).getDLW());
		if ((*it_rea).getTYR().getType() == TyrDistribution::distribution)
			tyr_dist[(*it_rea).getMT()] = (*it_rea).getTYR();
	}

	/* New array to hold the location of the tabulated angular distributions */
	vector<int> new_ldlw;
	/* Current number of cross sections */
	int dlw_size = energy_dist.size();
	new_ldlw.resize(dlw_size);

	int size_tyr = 0;
	std::map<int,TyrDistribution>::const_iterator it_tyr;
	for(it_tyr = tyr_dist.begin() ; it_tyr != tyr_dist.end() ; it_tyr++)
		size_tyr += (*it_tyr).second.getSize();

	/* Initial position (first xs is at "offset" 1) */
	new_ldlw[0] = size_tyr + 1;
	int last_size = energy_dist[0].getSize();
	int last_land = 1;
	for(int i = 1 ; i < dlw_size ; i++) {
		new_ldlw[i] = last_land + last_size;
		last_size = energy_dist[i].getSize();
		last_land = new_ldlw[i];
	}

	/* Set the new values of the XS location */
	LDLWBlock.setData(new_ldlw);
}

void DLWBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	LDLWBlock.updatePointers(nxs,jxs_old,jxs_new);

	nxs[NeutronTable::NR] = energy_dist.size();

	int size = 0;
	/* Get the size of each XS tabulated */
	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		size += (*it_ener).getSize();

	std::map<int,TyrDistribution>::const_iterator it_tyr;
	for(it_tyr = tyr_dist.begin() ; it_tyr != tyr_dist.end() ; it_tyr++)
		size += (*it_tyr).second.getSize();

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::DLW,size);
}

/* Dump the block, on a XSS stream */
void DLWBlock::dump(std::ostream& xss) {
	LDLWBlock.dump(xss);

	std::map<int,TyrDistribution>::const_iterator it_tyr;
	for(it_tyr = tyr_dist.begin() ; it_tyr != tyr_dist.end() ; it_tyr++)
		(*it_tyr).second.dump(xss);

	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		(*it_ener).dump(xss);
}

int DLWBlock::getSize() const {
	int size = 0;

	/* Get the size of each XS tabulated */
	vector<EnergyDistribution>::const_iterator it_ener;
	for(it_ener = energy_dist.begin() ; it_ener != energy_dist.end() ; it_ener++)
		size += (*it_ener).getSize();

	std::map<int,TyrDistribution>::const_iterator it_tyr;
	for(it_tyr = tyr_dist.begin() ; it_tyr != tyr_dist.end() ; it_tyr++)
		size += (*it_tyr).second.getSize();

	return size;
}
