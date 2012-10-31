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

#include "ESZBlock.hpp"
#include "../AceUtils.hpp"
#include "../NeutronTable.hpp"

using namespace std;
using namespace ACE;

ESZBlock::ESZBlock(const int nxs[nxs_size], const int jxs[jxs_size], const vector<double>& xss, ACETable* ace_table) : ACEBlock(xss,ace_table) {
	/* Length of each table */
	size_t table_length = nxs[NeutronTable::NES];
	/* Begin of ESZ block */
	setBegin(xss.begin() + (jxs[NeutronTable::ESZ] - 1));

	getXSS(energy,table_length);
	getXSS(sigma_t,table_length);
	getXSS(sigma_a,table_length);
	getXSS(sigma_e,table_length);
	getXSS(heat,table_length);
}

NRContainer& ESZBlock::reas() {return dynamic_cast<NeutronTable*>(ace_table)->getReactions();};

void ESZBlock::dump(ostream& xss) {
	putXSS(energy,xss);
	putXSS(sigma_t,xss);
	putXSS(sigma_a,xss);
	putXSS(sigma_e,xss);
	putXSS(heat,xss);
}

void ESZBlock::updateData() {
	/* Update the elastic cross section */
	CrossSection elastic_xs = reas().get_xs(2);
	sigma_e = elastic_xs.getData();

	CrossSection abs_xs = reas().get_xs(101); /* Disappearance (fission not included) */
	if (abs_xs.getData().size() > 0)
		sigma_a = abs_xs.getData();
	else
		sigma_a = CrossSection(sigma_e.size()).getData();

	/* Update the total cross section */
	CrossSection total_xs = reas().get_xs(1);
	sigma_t = total_xs.getData();
}

void ESZBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	/* Set the number of energies on the NXS array */
	nxs[NeutronTable::NES] = energy.size();

	/* Recalculate pointers on the JXS array */
	shiftJXSArray(jxs_old,jxs_new,NeutronTable::ESZ,getSize());
}

int ESZBlock::getType() const {
	return NeutronTable::ESZ;
};

int ESZBlock::getSize() const {
	return energy.size()*5;
};


ESZBlock::~ESZBlock() {
}

