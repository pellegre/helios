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

#include "NUBlock.hpp"
#include "../AceUtils.hpp"
#include "../NeutronTable.hpp"

using namespace std;
using namespace Ace;

NUBlock::NUBlock(const int nxs[nxs_size], const int jxs[jxs_size], const vector<double>& xss, AceTable* ace_table) : ACEBlock(xss,ace_table) {
	/* Check if there is a NU block on the table */
	int knu = jxs[NeutronTable::NU];
	if(knu) {
		/* Begin of NU block */
		setBegin(xss.begin() + (knu - 1));

		/* Get first value */
		int lnu;
		getXSS(lnu);

		/* Check with kind of nu data we are dealing */
		if(lnu < 0) {
			/* Both, prompt and total */
			int knu_prompt;
			getXSS(knu_prompt);

			if(knu_prompt == flag_pol)
				nu_data.push_back(new Polynomial(getPosition(),ace_table));
			else if (knu_prompt == flag_tab)
				nu_data.push_back(new Tabular(getPosition(),ace_table));
			else
				return;

			/* Shift iterator */
			setBegin(getPosition() + nu_data[0]->getSize());

			int knu_total;
			getXSS(knu_total);

			if(knu_total == flag_pol)
				nu_data.push_back(new Polynomial(getPosition(),ace_table));
			else if (knu_total == flag_tab)
				nu_data.push_back(new Tabular(getPosition(),ace_table));

		} else {
			/* Only one of the arrays */
			int knu = lnu;

			if(knu == flag_pol)
				nu_data.push_back(new Polynomial(getPosition(),ace_table));
			else if (knu == flag_tab)
				nu_data.push_back(new Tabular(getPosition(),ace_table));
		}
	}
}

NUBlock::Polynomial::Polynomial(std::vector<double>::const_iterator _it, AceTable* ace_table) : NUBlock::NuData(_it,ace_table) {
	getXSS(ncoef);
	getXSS(coef,ncoef);
}

void NUBlock::Polynomial::dump(std::ostream& xss) {
	putXSS(ncoef,xss);
	putXSS(coef,xss);
}

NUBlock::Tabular::Tabular(std::vector<double>::const_iterator _it, AceTable* ace_table) : NUBlock::NuData(_it,ace_table) {
	getXSS(nr);
	getXSS(nbt,nr);
	getXSS(aint,nr);
	getXSS(ne);
	getXSS(energies,ne);
	getXSS(nu,ne);
}

void NUBlock::Tabular::dump(std::ostream& xss) {
	putXSS(nr,xss);
	putXSS(nbt,xss);
	putXSS(aint,xss);
	putXSS(ne,xss);
	putXSS(energies,xss);
	putXSS(nu,xss);
}

/* Clone data */
vector<NUBlock::NuData*> NUBlock::clone() const {
	vector<NUBlock::NuData*> cloned;
	for(vector<NUBlock::NuData*>::const_iterator it = nu_data.begin() ; it != nu_data.end() ; ++it)
		cloned.push_back((*it)->clone());
	return cloned;
}

/* Dump the block, on a xss stream */
void NUBlock::dump(std::ostream& xss) {
	if(nu_data.size() == 1) {
		/* Only one NU table */
		putXSS(nu_data[0]->getType(),xss); /* Dump the type of table */
		nu_data[0]->dump(xss);             /* Dump the NU table */
	} else if (nu_data.size() == 2) {
		/* We have two tables */
		putXSS(- (nu_data[0]->getSize() + 1),xss); /* Dump the offset to the next NU table */

		putXSS(nu_data[0]->getType(),xss); /* Dump the type of table (prompt) */
		nu_data[0]->dump(xss);             /* Dump the NU table */

		putXSS(nu_data[1]->getType(),xss); /* Dump the type of table (total )*/
		nu_data[1]->dump(xss);             /* Dump the NU table */
	}
}

int NUBlock::getSize() const {
	std::vector<NuData*>::const_iterator it;
	int size = 0;

	for(it = nu_data.begin() ; it != nu_data.end() ; it++)
		size += (*it)->getSize();

	if(nu_data.size() == 2) {
		return (size + 3); /* NU data plus the first (negative) value plus two flag of the type of table */
	} else if (nu_data.size() == 1)
		return (size + 1);     /* NU data plus the flag of the type of table */

	return 0;
}

int NUBlock::getType() const {
	return NeutronTable::NU;
};

NUBlock::~NUBlock() {
	std::vector<NuData*>::const_iterator it;

	for(it = nu_data.begin() ; it != nu_data.end() ; it++)
		delete (*it);
}
