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
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REACTIONTEST_HPP_
#define REACTIONTEST_HPP_

#include <string>
#include <algorithm>
#include <omp.h>

#include "../../../Common/Common.hpp"
#include "../../../Material/AceTable/AceModule.hpp"
#include "../../../Material/AceTable/AceMaterial.hpp"
#include "../../../Material/AceTable/AceReader/Ace.hpp"
#include "../../../Material/AceTable/AceReader/AceUtils.hpp"
#include "../../../Material/AceTable/AceReader/Conf.hpp"
#include "../../../Material/AceTable/AceReaction/ElasticScattering.hpp"
#include "../../../Material/AceTable/AceReaction/FissionReaction.hpp"
#include "../../../Tallies/Histogram.hpp"

#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

class SimpleReactionTest : public ::testing::Test {
protected:
	SimpleReactionTest() {/* */}
	virtual ~SimpleReactionTest() {/* */}
	void SetUp() {/* */}
	void TearDown() {/* */}
};


TEST_F(SimpleReactionTest, CheckReaction) {
	using namespace std;
	using namespace Helios;
	using namespace Ace;
	using namespace AceReaction;

	McEnvironment* environment = new McEnvironment;
	string name = "92235.03c";

	vector<McObject*> ace_objects;
	ace_objects.push_back(new AceObject(name));

	/* Setup environment */
	environment->pushObjects(ace_objects.begin(), ace_objects.end());
	environment->setup();

	/* Get isotope from ACE reader */
	NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable(name));

	/* Get reactions (inelastic scattering) */
	ReactionContainer reactions = ace_table->getReactions();

	/* Inelastic scattering cross section (don't include fission and elastic scattering) */
	CrossSection inelastic_xs;

	/* Map of MT with cross sections */
	map<int,CrossSection> mt_map;

	for(ReactionContainer::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it) {
		/* Get angular distribution type */
		int angular_data = (*it).getAngular().getKind();

		/* If the reaction does not contains angular data, we reach the end "secondary" particle's reactions */
		if(angular_data == Ace::AngularDistribution::no_data) break;

		/* Get MT of the reaction */
		int mt = (*it).getMt();

		/* We shouldn't include elastic and fission here */
		if((mt < 18 || mt > 21) && mt != 38 && mt != 2) {
			/* Cross section */
			CrossSection xs = (*it).getXs();
			/* Sum this reaction */
			inelastic_xs = inelastic_xs + xs;
			/* Put the cross section on the map */
			mt_map[mt] = xs;
		}
	}

	/* Get energy grid */
	vector<double> energy_grid = ace_table->getEnergyGrid();

	/* Energy */
	double energy = 1;
	/* Interpolate on energy grid */
	size_t idx = upper_bound(energy_grid.begin(), energy_grid.end(), energy) - energy_grid.begin() - 1;
	double factor = (energy - energy_grid[idx]) / (energy_grid[idx + 1] - energy_grid[idx]);
	double inel_total =  factor * (inelastic_xs[idx + 1] - inelastic_xs[idx]) + inelastic_xs[idx];

	/* Reaction histogram */
	map<int,double> reaction_prob;

	for(map<int,CrossSection>::const_iterator it = mt_map.begin() ; it != mt_map.end() ; ++it) {
		/* Inelastic xs at this energy (total and from this reaction) */
		double inel_rea = factor * ((*it).second[idx + 1] - (*it).second[idx]) + (*it).second[idx];
		double prob = inel_rea / inel_total;
		/* Save */
		reaction_prob[(*it).first] = prob;
	}

	/* Sample reactions */
	size_t nsamples = 1000000000;
	/* Get isotope */
	AceIsotope* isotope = environment->getObject<AceModule,AceIsotope>(name)[0];
	/* Random number */
	Random random(1);
	/* Samples */
	map<int,double> reaction_samples;

	#pragma omp parallel
	{
		Random local_random(random);
		int size = omp_get_num_threads();
		int rank = omp_get_thread_num();
		local_random.jump(rank*nsamples/size);
		map<int,double> reaction_local_samples;

		#pragma omp for
		for(size_t  i = 0 ; i < nsamples ; ++i) {
			Energy energy_pair(0,energy);
			Reaction* rea = isotope->inelastic(energy_pair,local_random);
			int mt = rea->getId();
			reaction_local_samples[mt]++;
		}

		/* Sum contributions */
		#pragma omp critical
		{
			for(map<int,double>::const_iterator it = reaction_local_samples.begin() ;
					it != reaction_local_samples.end() ; ++it) reaction_samples[(*it).first] += (*it).second;
		}
	}

	/* Collect samples and check results */
	for(map<int,double>::iterator it = reaction_samples.begin() ; it != reaction_samples.end() ; ++it) {
		(*it).second /= (double)nsamples;
		/* Get difference */
		double error = 100.0* fabs(reaction_prob[(*it).first] - (*it).second) / reaction_prob[(*it).first];
		cout << setw(6) << (*it).first << setw(15) << scientific << (*it).second << setw(15)
				<< reaction_prob[(*it).first] << setw(15) << error << endl;
	}

	delete environment;
}

//TEST_F(SimpleReactionTest, ChanceFission) {
//	using namespace std;
//	using namespace Helios;
//	using namespace Ace;
//	using namespace AceReaction;
//
//	McEnvironment* environment = new McEnvironment;
//	string name = "92235.03c";
//
//	vector<McObject*> ace_objects;
//	ace_objects.push_back(new AceObject(name));
//
//	/* Setup environment */
//	environment->pushObjects(ace_objects.begin(), ace_objects.end());
//	environment->setup();
//
//	/* Get isotope from ACE reader */
//	NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable(name));
//
//	/* Get reactions (inelastic scattering) */
//	ReactionContainer reactions = ace_table->getReactions();
//
//	/* Inelastic scattering cross section (don't include fission and elastic scattering) */
//	CrossSection fission_xs;
//
//	/* Map of MT with cross sections */
//	map<int,CrossSection> mt_map;
//
//	for(ReactionContainer::const_iterator it = reactions.begin() ; it != reactions.end() ; ++it) {
//		/* Get angular distribution type */
//		int angular_data = (*it).getAngular().getKind();
//
//		/* If the reaction does not contains angular data, we reach the end "secondary" particle's reactions */
//		if(angular_data == Ace::AngularDistribution::no_data) break;
//
//		/* Get MT of the reaction */
//		int mt = (*it).getMt();
//
//		/* We shouldn't include elastic and fission here */
//		if((mt >= 19 && mt <= 21) || (mt == 38)) {
//			/* Cross section */
//			CrossSection xs = (*it).getXs();
//			/* Sum this reaction */
//			fission_xs = fission_xs + xs;
//			/* Put the cross section on the map */
//			mt_map[mt] = xs;
//		}
//	}
//
//	/* Get energy grid */
//	vector<double> energy_grid = ace_table->getEnergyGrid();
//
//	/* Energy */
//	double energy = 25;
//	/* Interpolate on energy grid */
//	size_t idx = upper_bound(energy_grid.begin(), energy_grid.end(), energy) - energy_grid.begin() - 1;
//	double factor = (energy - energy_grid[idx]) / (energy_grid[idx + 1] - energy_grid[idx]);
//	double fission_total =  factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
//
//	/* Reaction histogram */
//	map<int,double> reaction_prob;
//
//	for(map<int,CrossSection>::const_iterator it = mt_map.begin() ; it != mt_map.end() ; ++it) {
//		/* Inelastic XS at this energy (total and from this reaction) */
//		double inel_rea = factor * ((*it).second[idx + 1] - (*it).second[idx]) + (*it).second[idx];
//		double prob = inel_rea / fission_total;
//		/* Save */
//		reaction_prob[(*it).first] = prob;
//	}
//
//	/* Sample reactions */
//	size_t nsamples = 100000000;
//	/* Get isotope */
//	AceIsotope* isotope = environment->getObject<AceModule,AceIsotope>(name)[0];
//	/* Random number */
//	Random random(1);
//	/* Samples */
//	map<int,double> reaction_samples;
//
//	#pragma omp parallel
//	{
//		Random local_random(random);
//		int size = omp_get_num_threads();
//		int rank = omp_get_thread_num();
//		local_random.jump(rank*nsamples/size);
//		map<int,double> reaction_local_samples;
//
//		#pragma omp for
//		for(size_t  i = 0 ; i < nsamples ; ++i) {
//			Energy energy_pair(0,energy);
//			Reaction* rea = dynamic_cast<ChanceFission*>(isotope->fission())->sample(energy_pair, random);
//			int mt = rea->getId();
//			reaction_local_samples[mt]++;
//		}
//
//		/* Sum contributions */
//		#pragma omp critical
//		{
//			for(map<int,double>::const_iterator it = reaction_local_samples.begin() ;
//					it != reaction_local_samples.end() ; ++it) reaction_samples[(*it).first] += (*it).second;
//		}
//	}
//
//	/* Collect samples and check results */
//	for(map<int,double>::iterator it = reaction_samples.begin() ; it != reaction_samples.end() ; ++it) {
//		(*it).second /= (double)nsamples;
//		/* Get difference */
//		double error = 100.0* fabs(reaction_prob[(*it).first] - (*it).second) / reaction_prob[(*it).first];
//		cout << setw(6) << (*it).first << setw(15) << scientific << (*it).second << setw(15)
//				<< reaction_prob[(*it).first] << setw(15) << error << endl;
//	}
//
//	delete environment;
//}

#endif /* REACTIONTEST_HPP_ */
