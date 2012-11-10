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

#include "NeutronTable.hpp"
#include "Blocks/Blocks.hpp"
#include "AceUtils.hpp"


using namespace std;
using namespace Ace;

map<int,string> SetMT() {
	map<int,string> m;
	m[1]  = "(n,total) Neutron total";
	m[2]  = "(z,z0) Elastic scattering";
	m[3]  = "(z,nonelas) Nonelastic neutron";
	m[4]  = "(z,n) One neutron in exit channel";
	m[5]  = "(z,anything) Miscellaneous";
	m[10] = "(z,contin) Total continuum reaction";
	m[11] = "(z,2nd) Production of 2n and d";
	m[16] = "(z,2n) Production of 2n";
	m[17] = "(z,3n) Production of 3n";
	m[18] = "(z,fiss) Particle-induced fission";
	m[19] = "(z,f) First-chance fission";
	m[20] = "(z,nf) Second chance fission";
	m[21] = "(z,2nf) Third-chance fission";
	m[22] = "(z,na) Production of n and alpha";
	m[23] = "(z,n3a) Production of n and 3 alphas";
	m[24] = "(z,2na) Production of 2n and alpha";
	m[25] = "(z,3na) Production of 3n and alpha";
	m[27] = "(n,abs) Absorption";
	m[28] = "(z,np) Production of n and p";
	m[29] = "(z,n2a) Production of n and 2 alphas";
	m[30] = "(z,2n2a) Production of 2n and 2 alphas";
	m[32] = "(z,nd) Production of n and d";
	m[33] = "(z,nt) Production of n and t";
	m[34] = "(z,n3He) Production of n and He-3";
	m[35] = "(z,nd2a) Production of n, d, and alpha";
	m[36] = "(z,nt2a) Production of n, t, and 2 alphas";
	m[37] = "(z,4n) Production of 4n";
	m[38] = "(z,3nf) Fourth-chance fission";
	m[41] = "(z,2np) Production of 2n and p";
	m[42] = "(z,3np) Production of 3n and p";
	m[44] = "(z,n2p) Production of n and 2p";
	m[45] = "(z,npa) Production of n, p, and alpha";

	m[50] = "(z,n0) Production of n, ground state";
	for(int i = 51 ; i < 91 ; i++)
		m[i] = "(z,n"+ toString(i - 50) +") Production of n, excited state = " + toString(i - 50);
	m[91] = "(z,nc) Production of n in continuum";

	m[101] = "(n,disap) Neutron disappeareance";
	m[102] = "(z,gamma) Radiative capture";
	m[103] = "(z,p) Production of p";
	m[104] = "(z,d) Production of d";
	m[105] = "(z,t) Production of t";
	m[106] = "(z,3He) Production of He-3";
	m[107] = "(z,a) Production of alpha";
	m[108] = "(z,2a) Production of 2 alphas";
	m[109] = "(z,3a) Production of 3 alphas";
	m[111] = "(z,2p) Production of 2p";
	m[112] = "(z,pa) Production of p and alpha";
	m[113] = "(z,t2a) Production of t and 2 alphas";
	m[114] = "(z,d2a) Production of d and 2 alphas";
	m[115] = "(z,pd) Production of p and d";
	m[116] = "(z,pt) Production of p and t";
	m[117] = "(z,da) Production of d and a";
	m[151] = "Resonance Parameters";
	m[201] = "(z,Xn) Total neutron production";
	m[202] = "(z,Xgamma) Total gamma production";
	m[203] = "(z,Xp) Total proton production";
	m[204] = "(z,Xd) Total deuteron production";
	m[205] = "(z,Xt) Total triton production";
	m[206] = "(z,X3He) Total He-3 production";
	m[207] = "(z,Xa) Total alpha production";
	m[208] = "(z,Xpi+) Total pi+ meson production";
	m[209] = "(z,Xpi0) Total pi0 meson production";
	m[210] = "(z,Xpi-) Total pi- meson production";
	m[211] = "(z,Xmu+) Total anti-muon production";
	m[212] = "(z,Xmu-) Total muon production";
	m[213] = "(z,Xk+) Total positive kaon production";
	m[214] = "(z,Xk0long) Total long-lived neutral kaon production";
	m[215] = "(z,Xk0short) Total short-lived neutral kaon production";
	m[216] = "(z,Xk-) Total negative kaon production";
	m[217] = "(z,Xp-) Total anti-proton production";
	m[218] = "(z,Xn-) Total anti-neutron production";
	m[251] = "Average cosine of scattering angle";
	m[252] = "Average logarithmic energy decrement";
	m[253] = "Average xi^2/(2*xi)";
	m[444] = "Damage";
	m[451] = "Desciptive Data";
	m[452] = "Total Neutrons per Fission";
	m[454] = "Independent fission product yield";
	m[455] = "Delayed Neutron Data";
	m[456] = "Prompt Neutrons per Fission";
	m[457] = "Radioactive Decay Data";
	m[458] = "Energy Release Due to Fission";
	m[459] = "Cumulative Fission Product Yield";
	m[460] = "Delayed Photon Data";
	m[500] = "Total charged-particle stopping power";
	m[501] = "Total photon interaction";
	m[502] = "Photon coherent scattering";
	m[504] = "Photon incoherent scattering";
	m[505] = "Imaginary scattering factor";
	m[506] = "Real scattering factor";
	m[515] = "Pair production, electron field";
	m[516] = "Total pair production";
	m[517] = "Pair production, nuclear field";
	m[522] = "Photoelectric absorption";
	m[523] = "Photo-excitation cross section";
	m[526] = "Electro-atomic scattering";
	m[527] = "Electro-atomic bremsstrahlung";
	m[528] = "Electro-atomic excitation cross section";
	m[533] = "Atomic relaxation data";
	m[534] = "K (1s1/2) subshell";
	m[535] = "L1 (2s1/2) subshell";
	m[536] = "L2 (2p1/2) subshell";
	m[537] = "L3 (2p3/2) subshell";
	m[538] = "M1 (3s1/2) subshell";
	m[539] = "M2 (3p1/2) subshell";
	m[540] = "M3 (3p3/2) subshell";
	m[541] = "M4 (3d1/2) subshell";
	m[542] = "M5 (3d1/2) subshell";
	m[543] = "N1 (4s1/2) subshell";
	m[544] = "N2 (4p1/2) subshell";
	m[545] = "N3 (4p3/2) subshell";
	m[546] = "N4 (4d3/2) subshell";
	m[547] = "N5 (4d5/2) subshell";
	m[548] = "N6 (4f5/2) subshell";
	m[549] = "N7 (4f7/2) subshell";
	m[550] = "O1 (5s1/2) subshell";
	m[551] = "O2 (5p1/2) subshell";
	m[552] = "O3 (5p3/2) subshell";
	m[553] = "O4 (5d3/2) subshell";
	m[554] = "O5 (5d5/2) subshell";
	m[555] = "O6 (5f5/2) subshell";
	m[556] = "O7 (5f7/2) subshell";
	m[557] = "O8 (5g7/2) subshell";
	m[558] = "O9 (5g9/2) subshell";
	m[559] = "P1 (6s1/2) subshell";
	m[560] = "P2 (6p1/2) subshell";
	m[561] = "P3 (6p3/2) subshell";
	m[562] = "P4 (6d3/2) subshell";
	m[563] = "P5 (6d5/2) subshell";
	m[564] = "P6 (6f5/2) subshell";
	m[565] = "P7 (6f7/2) subshell";
	m[566] = "P8 (6g7/2) subshell";
	m[567] = "P9 (6g9/2) subshell";
	m[568] = "P10 (6h9/2) subshell";
	m[569] = "P11 (6h11/2) subshell";
	m[570] = "Q1 (7s1/2) subshell";
	m[571] = "Q2 (7p1/2) subshell";
	m[572] = "Q3 (7p3/2) subshell";

	m[600] = "(z,p0) Production of p, ground state";
	for(int i = 601 ; i < 649 ; i++)
		m[i] = "(z,p"+ toString(i - 600) +") Production of p, excited state = " + toString(i - 600);
	m[649] = "(z,pc) Production of p in continuum";

	m[650] = "(z,d0) Production of d, ground state";
	for(int i = 651 ; i < 699 ; i++)
		m[i] = "(z,d"+ toString(i - 650) +") Production of d, excited state = " + toString(i - 650);
	m[699] = "(z,dc) Production of d in continuum";

	m[700] = "(z,t0) Production of t, ground state";
	for(int i = 701 ; i < 749 ; i++)
		m[i] = "(z,t"+ toString(i - 700) +") Production of t, excited state = " + toString(i - 700);
	m[749] = "(z,tc) Production of t in continuum";

	m[750] = "(z,3He0) Production of 3He, ground state";
	for(int i = 751 ; i < 799 ; i++)
		m[i] = "(z,3He"+ toString(i - 750) +") Production of 3He, excited state = " + toString(i - 750);
	m[799] = "(z,3Hec) Production of 3He in continuum";

	m[800] = "(z,a0) Production of a, ground state";
	for(int i = 801 ; i < 849 ; i++)
		m[i] = "(z,a"+ toString(i - 800) +") Production of a, excited state = " + toString(i - 800);
	m[849] = "(z,ac) Production of a in continuum";

	return m;
}

map<int,string> NeutronTable::mts_reactions = SetMT();

static const string& tab = "   ";
NeutronTable::NeutronTable(const std::string& _table_name, const std::string& full_path, size_t address) :
	AceTable(_table_name,full_path,address), reactions(getName(), getAtomicRatio(), getTemperature()) {

	/* Generic double block */
	blocks.push_back(new ESZBlock(nxs,jxs,xss,this));
	if(jxs[NU])
		blocks.push_back(new NUBlock(nxs,jxs,xss,this));

	NRBlock* nr_block = 0;
	if(nxs[NeutronTable::NTR]) {
		blocks.push_back(new NRBlock(nxs,jxs,xss,this));
		nr_block = getBlock<NRBlock>();
	}

	blocks.push_back(new ANDBlock(nxs,jxs,xss,this));

	vector<int> tyrs;
	vector<int> mats;
	if(nxs[NeutronTable::NR]) {
		tyrs = nr_block->tyr_block.getData();
		mats = nr_block->mtr_block.getData();
		blocks.push_back(new DLWBlock(nxs,jxs,xss,this,tyrs,mats));
	}

	/* AND block */
	ANDBlock* and_block = getBlock<ANDBlock>();
	/* DLW block */
	DLWBlock* dlw_block = 0;
	if(nxs[NeutronTable::NR])
		dlw_block = getBlock<DLWBlock>();

	/* Set the energy grid */
	reactions.set_grid(getEnergyGrid());
	/* Create elastic scattering reaction */
	reactions.push_back(NeutronReaction(2,0.0,TyrDistribution(1),getElastic(),and_block->and_dist[0],EnergyDistribution(EnergyDistribution::no_data)));

	/* Set the reactions */
	int nrea = nxs[NeutronTable::NTR];

	if(nrea) {

		/* Number of secondary neutrons reactions (excluding elastic) */
		int nsec = nxs[NeutronTable::NR];

		for(int i = 0 ; i < nrea ; i++) {
			/* Reaction data */
			int mt = nr_block->mtr_block.getData()[i];
			double q = nr_block->lqr_block.getData()[i];
			int tyr = nr_block->tyr_block.getData()[i];

			if(i < nsec) {
				if(abs(tyr) > 100)
					reactions.push_back(NeutronReaction(mt,q,dlw_block->tyr_dist[mt],nr_block->sig_block.xs[i],
							and_block->and_dist[i+1],dlw_block->energy_dist[i]));
				else if(tyr == 19) {
					/* Fission */
					TyrDistribution tyr_distribution(tyr);
					NUBlock* nu_block = getBlock<NUBlock>();
					tyr_distribution.setFission(nu_block->getNuData());
					reactions.push_back(NeutronReaction(mt,q,tyr_distribution,nr_block->sig_block.xs[i],
							and_block->and_dist[i+1],dlw_block->energy_dist[i]));
				} else
					reactions.push_back(NeutronReaction(mt,q,TyrDistribution(tyr),nr_block->sig_block.xs[i],
							and_block->and_dist[i+1],dlw_block->energy_dist[i]));
			}
			else {
				reactions.push_back(NeutronReaction(mt,q,TyrDistribution(tyr),nr_block->sig_block.xs[i],
						           AngularDistribution(AngularDistribution::no_data),
						           EnergyDistribution(EnergyDistribution::no_data)));
			}
		}
	}

}

void NeutronTable::updateData() {
	reactions.update_xs();
	AceTable::updateData();
}

void NeutronTable::printTableInfo(std::ostream& out) const {
	out << tab << "[+] Continuous Energy Neutron Table " << endl;
	out << tab << "[@] Number of energies                  : " << nxs[NES] << endl;
	out << tab << "[@] Reactions (excluding elastic)       : " << nxs[NTR] << endl;
	out << tab << "[@] Reactions having secondary neutrons : " << nxs[NR] << endl;
	out << tab << "[@] Photon production reactions         : " << nxs[NTRP] << endl;
	out << tab << "[@] Delayed precursor families          : " << nxs[NPCR] << endl;
	out << tab << "[@] Fission                             : ";
	if(jxs[NU])
		 out << "PRESENT" << endl;
	else
		 out << "NOT PRESENT" << endl;

	ReactionContainer::const_iterator it_rea;

	out << tab << "[@] Neutron Reactions                   : " << endl;
	for(it_rea = reactions.begin() ; it_rea != reactions.end() ; it_rea++) {
		int mt = (*it_rea).getMt();
		if(mts_reactions.find(mt) != mts_reactions.end())
			out << tab << "[+] " << mts_reactions[mt] << " : " << endl;
		else
			out << tab << "[+] " << "Unknown ENDF reaction " << " : " <<  endl;
		(*it_rea).printInformation(out);
	}
}

const vector<double>& NeutronTable::getEnergyGrid() const {
	return getBlock<ESZBlock>()->energy;
}

CrossSection NeutronTable::getTotal() const {
	return CrossSection(1,getBlock<ESZBlock>()->sigma_t);
}

CrossSection NeutronTable::getAbsorption() const {
	return CrossSection(1,getBlock<ESZBlock>()->sigma_a);
}

CrossSection NeutronTable::getElastic() const {
	return CrossSection(1,getBlock<ESZBlock>()->sigma_e);
}


NeutronTable::~NeutronTable() { /* */ }
