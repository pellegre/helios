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

#include "EnergyDistribution.hpp"
#include "AceUtils.hpp"

using namespace std;
using namespace Ace;

EnergyDistribution::InterScheme::InterScheme(std::vector<double>::const_iterator& it) {
	getXSS(nr,it);
	getXSS(nbt,nr,it);
	getXSS(aint,nr,it);
}

void EnergyDistribution::InterScheme::dump(std::ostream& xss) const {
	putXSS(nr,xss);
	putXSS(nbt,xss);
	putXSS(aint,xss);
}

int EnergyDistribution::InterScheme::getSize() const {
	return (1 + nbt.size() + aint.size());
}

static map<int,string> init_laws() {
	map<int,string> m;
	m[1]  = "Tabular equiprobable energy bins";
	m[2]  = "Discrete photon energy";
	m[3]  = "Level scattering";
	m[4]  = "Continuous tabular distribution";
	m[5]  = "General evaporation spectrum";
	m[7]  = "Simple Maxwell fission spectrum";
	m[9]  = "Evaporation spectrum";
	m[11] = "Energy dependent Watt spectrum";
	m[22] = "Tabular linear functions";
	m[24] = "From UK Law 6";
	m[44] = "Kalbach-87 Formalism";
	m[61] = "Like 44, but tabular distribution instead of Kalbach-87";
	m[66] = "N-body phase space distribution";
	m[67] = "Laboratory angle-energy law";
	return m;
}

map<int,string> EnergyDistribution::energy_laws = init_laws();

EnergyDistribution::EnergyLaw* EnergyDistribution::EnergyLaw::createLaw(std::vector<double>::const_iterator it, int lnw, int law) {

	if(law == 1)
		return new EnergyDistribution::Law1(it,lnw);
	else if(law == 2)
		return new EnergyDistribution::Law2(it,lnw);
	else if(law == 3)
		return new EnergyDistribution::Law3(it,lnw);
	else if(law == 4)
		return new EnergyDistribution::Law4(it,lnw);
	else if(law == 5)
		return new EnergyDistribution::Law5(it,lnw);
	else if(law == 7)
		return new EnergyDistribution::Law7(it,lnw);
	else if(law == 9)
		return new EnergyDistribution::Law9(it,lnw);
	else if(law == 11)
		return new EnergyDistribution::Law11(it,lnw);
	else if(law == 22)
		return new EnergyDistribution::Law22(it,lnw);
	else if(law == 24)
		return new EnergyDistribution::Law24(it,lnw);
	else if(law == 44)
		return new EnergyDistribution::Law44(it,lnw);
	else if(law == 61)
		return new EnergyDistribution::Law61(it,lnw);
	else if(law == 66)
		return new EnergyDistribution::Law66(it,lnw);
	else if(law == 67)
		return new EnergyDistribution::Law67(it,lnw);

	return 0;
}

EnergyDistribution::EnergyDistribution(std::vector<double>::const_iterator it, std::vector<double>::const_iterator begin) {
	/* Get the law */
	int lnw;
	getXSS(lnw,it);
	int law;
	getXSS(law,it);
	laws.push_back(EnergyLaw::createLaw(it,lnw,law));
	while(lnw != 0) {
		it = begin + lnw - 1;
		getXSS(lnw,it);
		getXSS(law,it);
		laws.push_back(EnergyLaw::createLaw(it,lnw,law));
	}
	kind = data;
}

EnergyDistribution::EnergyDistribution(const EnergyDistribution& ed) {
	std::vector<EnergyLaw*>::const_iterator it_law;
	for(it_law = ed.laws.begin() ; it_law != ed.laws.end() ; it_law++)
		laws.push_back((*it_law)->clone());
	kind = ed.kind;
}

void EnergyDistribution::dump(std::ostream& xss) const {
	std::vector<EnergyLaw*>::const_iterator it_law;
	for(it_law = laws.begin() ; it_law != laws.end() ; it_law++)
		(*it_law)->dump(xss);
}

int EnergyDistribution::getSize() const {
	int size = 0 ;
	std::vector<EnergyLaw*>::const_iterator it_law = laws.begin();
	for( ; it_law != laws.end() ; it_law++)
		size += (*it_law)->getSize();
	return size;
}

std::vector<int> EnergyDistribution::getLaws() const {
	vector<int> nlaws;

	std::vector<EnergyLaw*>::const_iterator it_law;
	for(it_law = laws.begin() ; it_law != laws.end() ; it_law++)
		nlaws.push_back((*it_law)->getLaw());

	return nlaws;
}

EnergyDistribution::~EnergyDistribution() {
	std::vector<EnergyLaw*>::iterator it_law;
	for(it_law = laws.begin() ; it_law != laws.end() ; it_law++)
		delete((*it_law));
}

EnergyDistribution::EnergyLaw::EnergyLaw(std::vector<double>::const_iterator& it, int lnw) : lnw(lnw){
	set_common(it);
}

/* Dump this law */
void EnergyDistribution::EnergyLaw::dump(std::ostream& xss) const {
	putXSS(lnw,xss);
	putXSS(getLaw(),xss);
	putXSS(idat,xss);
	endf_int.dump(xss);
	putXSS(ne,xss);
	putXSS(energy,xss);
	putXSS(prob,xss);
	/* Dump the law array */
	dump_array(xss);
}

/* Get the total size */
int EnergyDistribution::EnergyLaw::getSize() const {
	return 4 + endf_int.getSize() + energy.size() + prob.size() + size_array();
}

void EnergyDistribution::EnergyLaw::set_common(std::vector<double>::const_iterator& it) {
	getXSS(idat,it);
	endf_int = InterScheme(it);
	getXSS(ne,it);
	getXSS(energy,ne,it);
	getXSS(prob,ne,it);
}

EnergyDistribution::Law1::Law1(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,ne,it);
	getXSS(net,it);
	eout.resize(nen);
	for(int i = 0 ; i < nen ; i++)
		getXSS(eout[i],net,it);
}

void EnergyDistribution::Law1::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(net,xss);
	for(int i = 0 ; i < nen ; i++)
		putXSS(eout[i],xss);
}

int EnergyDistribution::Law1::size_array() const {
 return (2 + int_sch.getSize() + ein.size() + eout.size()*net);
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law1::clone() const {

    Law1* ptr = new Law1(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->net = net;
    ptr->eout = eout;

    return ptr;
}

EnergyDistribution::Law2::Law2(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	getXSS(lp,it);
	getXSS(eg,it);
}

void EnergyDistribution::Law2::dump_array(std::ostream& xss) const {
	putXSS(lp,xss);
	putXSS(eg,xss);
}

int EnergyDistribution::Law2::size_array() const {
	return 2;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law2::clone() const {

    Law2* ptr = new Law2(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->lp = lp;
    ptr->eg = eg;

    return ptr;
}

EnergyDistribution::Law3::Law3(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	getXSS(ldat1,it);
	getXSS(ldat2,it);
}

void EnergyDistribution::Law3::dump_array(std::ostream& xss) const {
	putXSS(ldat1,xss);
	putXSS(ldat2,xss);
}

int EnergyDistribution::Law3::size_array() const {
	return 2;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law3::clone() const {

    Law3* ptr = new Law3(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->ldat1 = ldat1;
    ptr->ldat2 = ldat2;

    return ptr;
}

EnergyDistribution::Law4::Law4(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(loca,nen,it);

	for(int i = 0 ; i < nen ; i++) {
		EnergyData ed;
		getXSS(ed.intt,it);
		getXSS(ed.np,it);
		getXSS(ed.eout,ed.np,it);
		getXSS(ed.pdf,ed.np,it);
		getXSS(ed.cdf,ed.np,it);
		eout_dist.push_back(ed);
	}
}

void EnergyDistribution::Law4::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(loca,xss);

	for(int i = 0 ; i < nen ; i++) {
		putXSS(eout_dist[i].intt,xss);
		putXSS(eout_dist[i].np,xss);
		putXSS(eout_dist[i].eout,xss);
		putXSS(eout_dist[i].pdf,xss);
		putXSS(eout_dist[i].cdf,xss);
	}

}

int EnergyDistribution::Law4::size_array() const {
	int size = int_sch.getSize() + 1 + ein.size() + loca.size();

	for(int i = 0 ; i < nen ; i++)
		size += eout_dist[i].getSize();

	return size;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law4::clone() const {

    Law4* ptr = new Law4(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->loca = loca;
    ptr->eout_dist = eout_dist;

    return ptr;
}

EnergyDistribution::Law5::Law5(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(t,nen,it);
	getXSS(net,it);
	getXSS(x,net,it);
}

void EnergyDistribution::Law5::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(t,xss);
	putXSS(net,xss);
	putXSS(x,xss);
}

int EnergyDistribution::Law5::size_array() const {
	return (int_sch.getSize() + 2 + ein.size() + t.size() + x.size());
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law5::clone() const {

    Law5* ptr = new Law5(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->t = t;
    ptr->net = net;
    ptr->x = x;

    return ptr;
}

EnergyDistribution::Law7::Law7(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(t,nen,it);
	getXSS(u,it);
}

void EnergyDistribution::Law7::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(t,xss);
	putXSS(u,xss);
}

int EnergyDistribution::Law7::size_array() const {
	return (int_sch.getSize() + 2 + ein.size() + t.size());
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law7::clone() const {

    Law7* ptr = new Law7(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->t = t;
    ptr->u = u;

    return ptr;
}

EnergyDistribution::Law9::Law9(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(t,nen,it);
	getXSS(u,it);
}

void EnergyDistribution::Law9::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(t,xss);
	putXSS(u,xss);
}

int EnergyDistribution::Law9::size_array() const {
	return (int_sch.getSize() + 2 + ein.size() + t.size());
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law9::clone() const {

    Law9* ptr = new Law9(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->t = t;
    ptr->u = u;

    return ptr;
}

EnergyDistribution::Law11::Law11(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	inta = InterScheme(it);
	getXSS(nena,it);
	getXSS(eina,nena,it);
	getXSS(a,nena,it);

	intb = InterScheme(it);
	getXSS(nenb,it);
	getXSS(einb,nenb,it);
	getXSS(b,nenb,it);
	getXSS(u,it);
}

void EnergyDistribution::Law11::dump_array(std::ostream& xss) const {
	inta.dump(xss);
	putXSS(nena,xss);
	putXSS(eina,xss);
	putXSS(a,xss);

	intb.dump(xss);
	putXSS(nenb,xss);
	putXSS(einb,xss);
	putXSS(b,xss);
	putXSS(u,xss);
}

int EnergyDistribution::Law11::size_array() const {
	return (3 + inta.getSize() + intb.getSize() + eina.size() + einb.size() + a.size() + b.size());
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law11::clone() const {

    Law11* ptr = new Law11(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->inta = inta;
    ptr->nena = nena;
    ptr->eina = eina;
    ptr->a = a;

    ptr->intb = intb;
    ptr->nenb = nenb;
    ptr->einb = einb;
    ptr->b = b;
    ptr->u = u;

    return ptr;
}

EnergyDistribution::Law22::Law22(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(nen,it);
	getXSS(ein,it);
	getXSS(loca,it);
	for(int i = 0 ; i < nen ; i++) {
		EnergyData ed;
		getXSS(ed.nf,it);
		getXSS(ed.p,it);
		getXSS(ed.t,it);
		getXSS(ed.c,it);
		eout_dist.push_back(ed);
	}
}

void EnergyDistribution::Law22::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(loca,xss);

	for(int i = 0 ; i < nen ; i++) {
		putXSS(eout_dist[i].nf,xss);
		putXSS(eout_dist[i].p,xss);
		putXSS(eout_dist[i].t,xss);
		putXSS(eout_dist[i].c,xss);
	}
}

int EnergyDistribution::Law22::size_array() const {
	int size = (1 + int_sch.getSize() + ein.size() + loca.size());

	for(int i = 0 ; i < nen ; i++)
		size += eout_dist[i].getSize();

	return size;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law22::clone() const {

    Law22* ptr = new Law22(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->loca = loca;
    ptr->eout_dist = eout_dist;

    return ptr;
}

EnergyDistribution::Law24::Law24(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,ne,it);
	getXSS(net,it);
	t.resize(nen);
	for(int i = 0 ; i < nen ; i++)
		getXSS(t[i],net,it);
}

void EnergyDistribution::Law24::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(net,xss);
	for(int i = 0 ; i < nen ; i++)
		putXSS(t[i],xss);
}

int EnergyDistribution::Law24::size_array() const {
	 return (2 + int_sch.getSize() + ein.size() + t.size()*net);
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law24::clone() const {

    Law24* ptr = new Law24(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->net = net;
    ptr->t = t;

    return ptr;
}

EnergyDistribution::Law44::Law44(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(loca,nen,it);

	for(int i = 0 ; i < nen ; i++) {
		EnergyData ed;
		getXSS(ed.intt,it);
		getXSS(ed.np,it);
		getXSS(ed.eout,ed.np,it);
		getXSS(ed.pdf,ed.np,it);
		getXSS(ed.cdf,ed.np,it);
		getXSS(ed.r,ed.np,it);
		getXSS(ed.a,ed.np,it);
		eout_dist.push_back(ed);
	}
}

void EnergyDistribution::Law44::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(loca,xss);

	for(int i = 0 ; i < nen ; i++) {
		putXSS(eout_dist[i].intt,xss);
		putXSS(eout_dist[i].np,xss);
		putXSS(eout_dist[i].eout,xss);
		putXSS(eout_dist[i].pdf,xss);
		putXSS(eout_dist[i].cdf,xss);
		putXSS(eout_dist[i].r,xss);
		putXSS(eout_dist[i].a,xss);
	}

}

int EnergyDistribution::Law44::size_array() const {
	int size = int_sch.getSize() + 1 + ein.size() + loca.size();

	for(int i = 0 ; i < nen ; i++)
		size += eout_dist[i].getSize();

	return size;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law44::clone() const {

    Law44* ptr = new Law44(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->loca = loca;
    ptr->eout_dist = eout_dist;

    return ptr;
}

EnergyDistribution::Law61::EnergyData::EnergyData(std::vector<double>::const_iterator it) {
	getXSS(intt,it);
	/* Number of energies */
	int ne;
	getXSS(ne,it);
	/* Get arrays */
	getXSS(eout,ne,it);
	getXSS(pdf,ne,it);
	getXSS(cdf,ne,it);
	getXSS(lc,ne,it);

	for(int i = 0 ; i < ne ; i++) {
		if(lc[i] > 0) {
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
		} else
			adist.push_back(new AngularDistribution::Isotropic());
	}
}

EnergyDistribution::Law61::EnergyData::EnergyData(const EnergyData& ed) :
	intt(ed.intt), eout(ed.eout), pdf(ed.pdf), cdf(ed.cdf), lc(ed.lc) {
	vector<AngularDistribution::AngularArray*>::const_iterator it;
	for(it = ed.adist.begin() ; it != ed.adist.end() ; it++)
		adist.push_back((*it)->clone());
}

int EnergyDistribution::Law61::EnergyData::getSize() const {
	int size = 0;
	size += 4 * eout.size() + 2;
	vector<AngularDistribution::AngularArray*>::const_iterator it;
	for(it = adist.begin() ; it != adist.end() ; it++)
		size += (*it)->size();
	return size;
};

void EnergyDistribution::Law61::EnergyData::dump(std::ostream& xss) const {
	putXSS(intt,xss);
	putXSS((int)eout.size(),xss);
	putXSS(eout,xss);
	putXSS(pdf,xss);
	putXSS(cdf,xss);
	putXSS(lc,xss);
	vector<AngularDistribution::AngularArray*>::const_iterator it;
	for(it = adist.begin() ; it != adist.end() ; it++)
		(*it)->dump(xss);
};

EnergyDistribution::Law61::EnergyData::~EnergyData() {
	vector<AngularDistribution::AngularArray*>::iterator it;
	for(it = adist.begin() ; it != adist.end() ; it++)
		delete (*it);
}

EnergyDistribution::Law61::Law61(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	int_sch = InterScheme(it);
	getXSS(nen,it);
	getXSS(ein,nen,it);
	getXSS(loca,nen,it);

	for(size_t i = 0 ; i < loca.size() ; i++) {
		/* Push the angular distribution into the container */
		eout_dist.push_back(EnergyData(it));
		it += eout_dist[i].getSize();
	}

}

void EnergyDistribution::Law61::dump_array(std::ostream& xss) const {
	int_sch.dump(xss);
	putXSS(nen,xss);
	putXSS(ein,xss);
	putXSS(loca,xss);

	vector<EnergyData>::const_iterator it;
	for(it = eout_dist.begin() ; it != eout_dist.end() ; it++)
		(*it).dump(xss);
}

int EnergyDistribution::Law61::size_array() const {
	int size = int_sch.getSize() + 1 + ein.size() + loca.size();

	for(int i = 0 ; i < nen ; i++)
		size += eout_dist[i].getSize();

	return size;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law61::clone() const {

    Law61* ptr = new Law61(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->int_sch = int_sch;
    ptr->nen = nen;
    ptr->ein = ein;
    ptr->loca = loca;
    ptr->eout_dist = eout_dist;

    return ptr;
}

EnergyDistribution::Law66::Law66(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {
	getXSS(npxs,it);
	getXSS(ap,it);
}

void EnergyDistribution::Law66::dump_array(std::ostream& xss) const {
	putXSS(npxs,xss);
	putXSS(ap,xss);
}

int EnergyDistribution::Law66::size_array() const {
	return 2;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law66::clone() const {

    Law66* ptr = new Law66(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    ptr->npxs = npxs;
    ptr->ap = ap;

    return ptr;
}

EnergyDistribution::Law67::Law67(std::vector<double>::const_iterator it, int lnw) : EnergyLaw(it,lnw) {

}

void EnergyDistribution::Law67::dump_array(std::ostream& xss) const {

}

int EnergyDistribution::Law67::size_array() const {
	return 0;
}

EnergyDistribution::EnergyLaw* EnergyDistribution::Law67::clone() const {

    Law67* ptr = new Law67(lnw,idat,endf_int.nr,endf_int.nbt,endf_int.aint,ne,energy,prob);

    return ptr;
}
