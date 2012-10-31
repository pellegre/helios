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

#include "ReactionContainer.hpp"
#include "AceUtils.hpp"

using namespace std;
using namespace Ace;

ReactionContainer::iterator ReactionContainer::get_mt(int mt) {
	ReactionContainer::iterator it = rea_cont.begin();
	while(it != rea_cont.end()) {
		if ((*it).getMT() == mt)
			return it;
		it++;
	}
	return it;
}

ReactionContainer::const_iterator ReactionContainer::get_mt(int mt) const {
	ReactionContainer::const_iterator it = rea_cont.begin();
	while(it != rea_cont.end()) {
		if ((*it).getMT() == mt)
			return it;
		it++;
	}
	return it;
}

CrossSection ReactionContainer::sum_mts(const std::string& range) const {
	if(rea_cont.size() == 0) return CrossSection();

	CrossSection xs_ret;

	ReactionContainer::const_iterator it_rea;
	set<int> mt_numbers = getNumbers(range);

	for(it_rea = rea_cont.begin(); it_rea != rea_cont.end() ; it_rea++) {
		if(mt_numbers.find((*it_rea).getMT()) != mt_numbers.end())
			xs_ret = xs_ret + (*it_rea).getXS();
	}

	return xs_ret;
}

bool ReactionContainer::check_any(const std::string& mts) const {
	ReactionContainer::const_iterator it_rea;
	set<int> mt_numbers = getNumbers(mts);

	for(it_rea = rea_cont.begin(); it_rea != rea_cont.end() ; it_rea++) {
		if(mt_numbers.find((*it_rea).getMT()) == mt_numbers.end())
			return false;
	}

	return true;
}

bool ReactionContainer::check_all(const std::string& mts) const {
	ReactionContainer::const_iterator it_rea;
	set<int> mt_numbers = getNumbers(mts);

	for(it_rea = rea_cont.begin(); it_rea != rea_cont.end() ; it_rea++) {
		if(mt_numbers.find((*it_rea).getMT()) != mt_numbers.end())
			return true;
	}

	return false;
}

std::string ReactionContainer::proton_xs() const {
	string proton = "";
	if(get_mt(103) == end() || check_any("600-649"))
		proton = "600-649";
	else
		proton = "103";
	return proton;
}

std::string ReactionContainer::deuteron_xs() const {
	string deuteron = "";
	if(get_mt(104) == end() || check_any("650-699"))
		deuteron = "650-699";
	else
		deuteron = "104";
	return deuteron;
}

std::string ReactionContainer::triton_xs() const {
	string triton = "";
	if(get_mt(105) == end() || check_any("700-749"))
		triton = "700-749";
	else
		triton = "105";
	return triton;
}

std::string ReactionContainer::he_xs() const {
	string he;
	if(get_mt(106) == end() || check_any("750-799"))
		he = "750-799";
	else
		he = "106";
	return he;
}

std::string ReactionContainer::alpha_xs() const {
	string alpha = "";
	if(get_mt(107) == end() || check_any("750-799"))
		alpha = "800-849";
	else
		alpha = "107";
	return alpha;
}

string ReactionContainer::fission_xs() const {
	/* Fission cross section */
	string fission = "";
	if(get_mt(18) == end() || check_all("19-21,38"))
		fission = "19-21,38";
	else
		fission = "18";
	return fission;
}

string ReactionContainer::disapp_xs() const {
	/* Create the absorption cross section (fission not included) */
	string total_abs = string("102,108-117") + "," + proton_xs() + "," + deuteron_xs() + "," + triton_xs() + "," + he_xs() + "," + alpha_xs();

	return total_abs;
}

std::string ReactionContainer::nonelastic_xs() const {
	return exitc_xs() + "," + "5,11,16-17,22-26,28-37,41-42,44-45" + "," +fission_xs() + "," + disapp_xs();
}

std::string ReactionContainer::exitc_xs() const {
	/* Fission cross section */
	string exitc = "";
	if(get_mt(4) == end() || check_any("50-91"))
		exitc = "50-91";
	else
		exitc = "4";
	return exitc;
}

string ReactionContainer::elastic_xs() const {
	/* Elastic cross section */
	return "2";
}

CrossSection ReactionContainer::get_xs(int mt) const {
	if(mt == 1)
		return sum_mts(nonelastic_xs() + "," + elastic_xs());
	else if(mt == 2)
		return sum_mts(elastic_xs());
	else if(mt == 3)
		return sum_mts(nonelastic_xs());
	else if(mt == 4)
		return sum_mts(exitc_xs());
	else if(mt == 27)
		return sum_mts(disapp_xs() + "," + fission_xs());
	else if(mt == 101)
		return sum_mts(disapp_xs());
	else if(mt == 18)
		return sum_mts(fission_xs());
	else if(mt == 103)
		return sum_mts(proton_xs());
	else if(mt == 104)
		return sum_mts(deuteron_xs());
	else if(mt == 105)
		return sum_mts(triton_xs());
	else if(mt == 106)
		return sum_mts(he_xs());
	else if(mt == 107)
		return sum_mts(alpha_xs());

	return sum_mts(toString(mt));
}

void ReactionContainer::update_xs() {
	ReactionContainer::iterator it;

	for(it = rea_cont.begin() ; it != rea_cont.end() ; it++) {
		int mt = (*it).getMT();

		if(mt == 1)
			(*it).getXS() = sum_mts(nonelastic_xs() + "," + elastic_xs());
		else if(mt == 3)
			(*it).getXS() = sum_mts(nonelastic_xs());
		else if(mt == 4)
			(*it).getXS() = sum_mts(exitc_xs());
		else if(mt == 27)
			(*it).getXS() = sum_mts(disapp_xs() + "," + fission_xs());
		else if(mt == 101)
			(*it).getXS() = sum_mts(disapp_xs());
		else if(mt == 18)
			(*it).getXS() = sum_mts(fission_xs());
		else if(mt == 103)
			(*it).getXS() = sum_mts(proton_xs());
		else if(mt == 104)
			(*it).getXS() = sum_mts(deuteron_xs());
		else if(mt == 105)
			(*it).getXS() = sum_mts(triton_xs());
		else if(mt == 106)
			(*it).getXS() = sum_mts(he_xs());
		else if(mt == 107)
			(*it).getXS() = sum_mts(alpha_xs());

	}
}
