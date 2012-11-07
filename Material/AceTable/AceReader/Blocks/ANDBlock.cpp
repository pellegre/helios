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

#include "ANDBlock.hpp"
#include "../AceUtils.hpp"

using namespace std;
using namespace Ace;

ANDBlock::ANDBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table)
    : ACEBlock(xss,ace_table), LANDBlock(nxs,jxs,xss,nxs[NeutronTable::NR] + 1,ace_table) {

	/* Get the number of MT numbers (i.e. of tabulated cross sections) */
	int nmt = nxs[NeutronTable::NR] + 1;

	/* Get the location data */
	vector<int> land_data = LANDBlock.getData();

	for(int i = 0 ; i < nmt ; i++) {
		if(land_data[i] == 0) {
			/* Create angular distribution */
			AngularDistribution ad(AngularDistribution::isotropic);
			/* Push the angular distribution into the container */
			and_dist.push_back(ad);
		} else if(land_data[i] == -1) {
			/* Create angular distribution */
			AngularDistribution ad(AngularDistribution::law44);
			/* Push the angular distribution into the container */
			and_dist.push_back(ad);
		} else {
			/* Location in the table */
			int loca = jxs[NeutronTable::AND] + land_data[i] - 1;
			/* Set the start point on the XSS table */
			setBegin(xss.begin() + (loca - 1));
			/* Push the angular distribution into the container */
			and_dist.push_back(AngularDistribution(getPosition()));
		}
	}
}

ReactionContainer& ANDBlock::reas() const {return dynamic_cast<NeutronTable*>(ace_table)->getReactions();};

void ANDBlock::updateData() {
	and_dist.clear();

	/* The first one is the elastic reaction */
	and_dist.push_back(reas()[0].getAngular());

	ReactionContainer::iterator it_rea;

	for(it_rea = reas().begin() + 1; it_rea != reas().end() ; it_rea++) {
		/* The first reaction with no data for the angular distribution defines the end of the NR array */
		if((*it_rea).getAngular().getKind() == AngularDistribution::no_data) break;
		and_dist.push_back((*it_rea).getAngular());
	}

	/* New array to hold the location of the tabulated angular distributions */
	vector<int> new_land;
	/* Current number of cross sections */
	int and_size = and_dist.size();
	new_land.resize(and_size);

	/* Initial position (first xs is at "offset" 1) */
	new_land[0] = 1;
	int last_size = and_dist[0].getSize();
	int last_land = 1;
	for(int i = 1 ; i < and_size ; i++) {
		if(and_dist[i].getKind() == AngularDistribution::isotropic)
			new_land[i] = 0;
		else if(and_dist[i].getKind() == AngularDistribution::law44)
			new_land[i] = -1;
		else {
			new_land[i] = last_land + last_size;
			last_size = and_dist[i].getSize();
			last_land = new_land[i];
		}
	}

	/* Set the new values of the XS location */
	LANDBlock.setData(new_land);
}

void ANDBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	LANDBlock.updatePointers(nxs,jxs_old,jxs_new);

	nxs[NeutronTable::NR] = and_dist.size() - 1;

	int size = 0;
	/* Get the size of each XS tabulated */
	vector<AngularDistribution>::const_iterator it;
	for(it = and_dist.begin() ; it != and_dist.end() ; it++)
		size += (*it).getSize();

	shiftJXSArray(jxs_old,jxs_new,NeutronTable::AND,size);
}

/* Dump the block, on a XSS stream */
void ANDBlock::dump(std::ostream& xss) {
	LANDBlock.dump(xss);
	vector<AngularDistribution>::iterator it;
	for(it = and_dist.begin() ; it != and_dist.end() ; it++)
		(*it).dump(xss);
}

int ANDBlock::getSize() const {
	/* LSIG block size */
	int size = 0;

	/* Get the size of each XS tabulated */
	vector<AngularDistribution>::const_iterator it;
	for(it = and_dist.begin() ; it != and_dist.end() ; it++)
		size += (*it).getSize();

	return size;
}
