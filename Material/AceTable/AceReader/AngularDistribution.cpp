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

#include "AngularDistribution.hpp"
#include "Utils.hpp"

using namespace std;
using namespace ACE;


AngularDistribution::AngularDistribution(const AngularDistribution& ad) : energy(ad.energy), lc(ad.lc) {
	vector<AngularArray*>::const_iterator it;
	for(it = ad.adist.begin() ; it != ad.adist.end() ; it++)
		adist.push_back((*it)->clone());
	kind = ad.kind;
}

AngularDistribution::AngularDistribution(std::vector<double>::const_iterator it) {
	/* Number of energies */
	int ne;
	getXSS(ne,it);
	/* Get arrays */
	getXSS(energy,ne,it);
	getXSS(lc,ne,it);

	for(int i = 0 ; i < ne ; i++) {
		if(lc[i] < 0) {
			int iflag;                 /* 1 = histogram, 2 = lin-lin */
			int np;
			std::vector<double> csout; /* Cosine scattering angular grid */
			std::vector<double> pdf;   /* Probability density function */
			std::vector<double> cdf;   /* Cumulative density function */
			getXSS(iflag,it);
			getXSS(np,it);
			getXSS(csout,np,it);
			getXSS(pdf,np,it);
			getXSS(cdf,np,it);
			adist.push_back(new AngularDistribution::Tabular(iflag,csout,pdf,cdf));
		} else if (lc[i] > 0) {
			std::vector<double> bins;
			getXSS(bins,33,it);
			adist.push_back(new AngularDistribution::EquiBins(bins));
		} else
			adist.push_back(new AngularDistribution::Isotropic());
	}

	kind = data;
}

void AngularDistribution::dump(ostream& xss) const {
	if(kind == data) {
		putXSS((int)energy.size(),xss);
		putXSS(energy,xss);
		putXSS(lc,xss);
		vector<AngularArray*>::const_iterator it;
		for(it = adist.begin() ; it != adist.end() ; it++)
			(*it)->dump(xss);
	}
}

int AngularDistribution::getSize() const {
	int size = 0;
	if(kind == data) {
		size += 2 * energy.size() + 1;
		vector<AngularArray*>::const_iterator it;
		for(it = adist.begin() ; it != adist.end() ; it++)
			size += (*it)->size();
	}
	return size;
}

void AngularDistribution::EquiBins::dump(std::ostream& out) const {putXSS(bins,out);};
void AngularDistribution::Tabular::dump(std::ostream& out) const {putXSS(iflag,out);putXSS((int)csout.size(),out);putXSS(csout,out);putXSS(pdf,out);putXSS(cdf,out);};

AngularDistribution::~AngularDistribution() {
	vector<AngularArray*>::iterator it;
	for(it = adist.begin() ; it != adist.end() ; it++)
		delete (*it);
}

