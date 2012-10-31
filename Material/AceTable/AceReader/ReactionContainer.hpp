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

#ifndef REACTIONCONTAINER_HPP_
#define REACTIONCONTAINER_HPP_

#include <iostream>
#include <string>
#include <vector>

#include "CrossSection.hpp"
#include "NeutronReaction.hpp"

namespace Ace {

class ReactionContainer {
	/* --- Information about the isotope */

	/* Name of the table */
	std::string table_name;
	/* Atomic weight ratio */
	double aweight;
	/* Temperature at which the data were processed (in MeV) */
	double temperature;

	/* Energy grid where the XS of the reactions are evaluated */
	std::vector<double> energy;

	/* Hold the neutron reactions on a STL vector (all of them have to be on the same grid) */
	std::vector<NeutronReaction> rea_cont;

	/* Cross section definitions, according to ENDF */
	std::string disapp_xs() const;
	std::string nonelastic_xs() const;
	std::string elastic_xs() const;
	std::string fission_xs() const;

	/* Production of one neutron in the exit channel */
	std::string exitc_xs() const;

	/* Production cross sections */
	std::string proton_xs() const;
	std::string deuteron_xs() const;
	std::string triton_xs() const;
	std::string he_xs() const;
	std::string alpha_xs() const;

	/* Update cross sections: The XS that are sums of other MTs are updated */
	void update_xs();

public:

	friend class NeutronTable;

	ReactionContainer(const std::string& table_name, const double& aweight, const double& temperature) :
				 table_name(table_name), aweight(aweight), temperature(temperature) {/* */};
	ReactionContainer(const std::string& table_name, const double& aweight, const double& temperature, const std::vector<double>& energy) :
				table_name(table_name), aweight(aweight), temperature(temperature), energy(energy) { /* */ };
	ReactionContainer(const std::string& table_name, const double& aweight, const double& temperature,
			    const std::vector<double>& energy, const std::vector<NeutronReaction>& rea_cont) :
				table_name(table_name), aweight(aweight), temperature(temperature), energy(energy), rea_cont(rea_cont) { /* */ };

	NeutronReaction& operator[](size_t x) {return rea_cont[x];}

	/* Iterators to this class */
	typedef std::vector<NeutronReaction>::iterator iterator;
	typedef std::vector<NeutronReaction>::const_iterator const_iterator;

	/* Iterators */
	iterator begin() {return rea_cont.begin();};
	const_iterator begin() const {return rea_cont.begin();};
	iterator end() {return rea_cont.end();};
	const_iterator end() const {return rea_cont.end();};

	/* Size of the container (i.e. number of reactions) */
	size_t size() const {return rea_cont.size();};
	/* Get the energy grid of this container */
	std::vector<double> get_grid() const {return energy;};
	/* Set the energy grid (without recalculating the reactions XS) */
	void set_grid(const std::vector<double>& _energy) {energy = _energy;};

	/* Push a new reaction on the container */
	void push_back(const NeutronReaction& rea) {rea_cont.push_back(rea);};

	/* Get a iterator to a reaction with a number equal to MT (if not, return a reference to end) */
	iterator get_mt(int mt);
	const_iterator get_mt(int mt) const;

	/* Sum a range of MTs numbers, and returns the XS */
	CrossSection sum_mts(const std::string& range) const;

	/* This function calculates the XS of a MT reaction according to the ENDF specification */
	CrossSection get_xs(int mt) const;

	/* This function checks if ANY of the MTs provided is on the container */
	bool check_any(const std::string& mts) const;

	/* This function checks if ALL of the MTs provided is on the container */
	bool check_all(const std::string& mts) const;

	/* Get name of the table that contains this reactions */
	std::string name() const {return table_name;}

	/* Get atomic weight ratio of the isotope */
	double awr() const {return aweight;}

	/* Get temperature (this is in Mevs) of the isotope */
	double temp() const {return temperature;}

	virtual ~ReactionContainer() {/* */};
};

} /* namespace ACE */
#endif /* REACTIONCONTAINER_HPP_ */
