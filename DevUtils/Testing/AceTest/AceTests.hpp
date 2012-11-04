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

#ifndef ACETESTS_HPP_
#define ACETESTS_HPP_

#include <string>
#include <algorithm>
#include <omp.h>

#include "../../../Common/Common.hpp"
#include "../../../Material/AceTable/AceModule.hpp"
#include "../../../Material/AceTable/AceMaterial.hpp"
#include "../../../Material/AceTable/AceReader/Ace.hpp"
#include "../../../Material/AceTable/AceReader/AceUtils.hpp"
#include "../../../Material/AceTable/AceReader/Conf.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

class SimpleAceTest : public ::testing::Test {
protected:
	SimpleAceTest() {
		using namespace Ace;
		using namespace std;
		/* Library to check */
		std::string library = "03c";
		std::string xsdir = Conf::DATAPATH + "/xsdir";

		/* Container of isotopes */
		/* Open XSDIR file*/
		ifstream is(xsdir.c_str());
		string str="";
		if (is.is_open()) {
			while ( is.good() ) {
				getline(is,str);
				if (iStringCompare(str,"directory")) break;
			}
			while ( !is.eof() ) {
				getline(is,str);
				/* Obtain information for construct an ACETable Object */
				if ( str.find(library) != string::npos ) {
					std::istringstream s(str);
					string t;
					s >> t;
					std::remove_if(t.begin(), t.end(), ::isspace);
					isotopes.push_back(t);
				}
			}
		}

	}
	virtual ~SimpleAceTest() {/* */}
	void SetUp() {/* */}
	void TearDown() {/* */}

	std::vector<std::string> isotopes;

};

//TEST_F(SimpleAceTest, SumReactions) {
//	using namespace std;
//	using namespace Ace;
//
//	/* Library to check */
//	std::string library = "c";
//	std::string xsdir = Conf::DATAPATH + "/xsdir";
//
//	/* Container of isotopes */
//	vector<string> isotopes;
//	/* Open XSDIR file*/
//	ifstream is(xsdir.c_str());
//	string str="";
//	if (is.is_open()) {
//		while ( is.good() ) {
//			getline(is,str);
//			if (iStringCompare(str,"directory")) break;
//		}
//		while ( !is.eof() ) {
//			getline(is,str);
//			/* Obtain information for construct an ACETable Object */
//			if ( str.find(library) != string::npos ) {
//				std::istringstream s(str);
//				string t;
//				s >> t;
//				std::remove_if(t.begin(), t.end(), ::isspace);
//				isotopes.push_back(t);
//			}
//		}
//	}
//
//	for(vector<string>::const_iterator it = isotopes.begin() ; it != isotopes.end() ; ++it) {
//		/* Get table */
//		NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable((*it)));
//
//		/* Check cross section MTs calculations */
//		CrossSection old_st = ace_table->getTotal();
//		CrossSection old_el = ace_table->getElastic();
//		CrossSection old_ab = ace_table->getAbsorption();
//
//		/* Get original reactions */
//		ReactionContainer old_rea = ace_table->getReactions();
//
//		ace_table->updateBlocks();
//
//		/* Get updated reactions */
//		ReactionContainer new_rea = ace_table->getReactions();
//
//		Helios::Log::bok() << " - Checking " << new_rea.name() << Helios::Log::crst <<
//				" (awr = " << setw(9) << new_rea.awr() << " , temp = " << setw(9) << new_rea.temp() << ") " << Helios::Log::endl;
//
//		/* Check MAIN cross sections */
//		double max_total = checkXS(old_st,ace_table->getTotal());
//		double max_ela = checkXS(old_el,ace_table->getElastic());
//		double max_abs = checkXS(old_ab,ace_table->getAbsorption());
//		double _max_diff = max(max_total,max_abs);
//		double max_diff = max(_max_diff,max_ela);
//		EXPECT_NEAR(0.0,max_diff,5e8*numeric_limits<double>::epsilon());
//
//		size_t nrea = new_rea.size();
//
//		for(size_t i = 0 ; i < nrea ; i++) {
//			double diff = checkXS(old_rea[i].getXS(),new_rea[i].getXS());
//			EXPECT_NEAR(0.0,diff,5e8*numeric_limits<double>::epsilon());
//		}
//
//		delete ace_table;
//	}
//
//}

class AceModuleTest : public SimpleAceTest {
protected:
	AceModuleTest() {/* */}
	virtual ~AceModuleTest() {/* */}

	void SetUp() {
		/* Environment */
		environment = new Helios::McEnvironment();
	}

	void TearDown() {
		delete environment;
	}

	void checkProbs(size_t begin, size_t end) {
		using namespace Helios;
		using namespace Ace;
		using namespace std;

		double eps = 5e9*numeric_limits<double>::epsilon();
		cout << "Using epsilon = " << scientific << eps << endl;

		vector<McObject*> ace_objects;
		vector<string> test_isotopes;

		/* Number of isotopes */
		for(size_t i = begin ; i < end; ++i) {
			string name = isotopes[i];
			test_isotopes.push_back(name);
			ace_objects.push_back(new AceObject(name));
		}

		/* Setup environment */
		environment->pushObjects(ace_objects.begin(), ace_objects.end());
		environment->setup();

		/* Number of random energies */
		size_t nrandom = 100;

		for(size_t i = 0 ; i < test_isotopes.size() ; ++i) {
			string name = test_isotopes[i];
			Log::bok() << " - Checking probabilities for" << name << Log::endl;
			/* Get isotope from environment */
			AceIsotope* iso = environment->getObject<AceModule,AceIsotope>(name)[0];
			/* Get table from file */
			NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable(name));

			/* Get disappearance */
			CrossSection dissap_xs = ace_table->getAbsorption();
			/* Get fission cross section */
			CrossSection fission_xs = ace_table->getReactions().get_xs(18);
			/* Calculate absorption cross section */
			CrossSection absorption_xs = dissap_xs + fission_xs;
			/* Get total cross section */
			CrossSection total_xs = ace_table->getTotal();

			Log::bok() << Log::ident(1) << " - Uniform sampling " << Log::endl;
			/* Sample inside the energy range of the isotope */
			for(size_t j = 0 ; j < nrandom ; ++j) {

				/* Get energy grid */
				vector<double> energy_grid = ace_table->getEnergyGrid();

				/* Get random energy */
				double energy = randomNumber(energy_grid[0], energy_grid[energy_grid.size() - 1]);
				Energy pair_energy(0,energy);

				/* Interpolate on energy grid */
				size_t idx = upper_bound(energy_grid.begin(), energy_grid.end(), energy) - energy_grid.begin() - 1;
				double factor = (energy - energy_grid[idx]) / (energy_grid[idx + 1] - energy_grid[idx]);

				/* Get interpolated cross sections */
				double sigma_t = factor * (total_xs[idx + 1] - total_xs[idx]) + total_xs[idx];
				double sigma_a = factor * (absorption_xs[idx + 1] - absorption_xs[idx]) + absorption_xs[idx];

				/* Check the total cross section */
				double rel = fabs((sigma_t - iso->getTotalXs(pair_energy)) / sigma_t);
				EXPECT_NEAR(0.0,rel,eps);

				/* Get probabilities */
				double abs_prob = sigma_a / sigma_t;
				double expected_abs = iso->getAbsorptionProb(pair_energy);

				/* Check against interpolated values */
				EXPECT_NEAR(abs_prob,expected_abs,eps);

				/* Check fission if the isotope is fissile */
				if(iso->isFissile()) {
					double sigma_f = factor * (fission_xs[idx + 1] - fission_xs[idx]) + fission_xs[idx];
					double fis_prob = sigma_f / sigma_t;
					double expected_fis = iso->getFissionProb(pair_energy);
					/* Check against interpolated values */
					EXPECT_NEAR(fis_prob,expected_fis,eps);
				}

			}

			Log::bok() << Log::ident(1) << " - Low sampling " << Log::endl;

			/* Sample lower values */
			for(size_t j = 0 ; j < nrandom ; ++j) {

				/* Get energy grid */
				vector<double> energy_grid = ace_table->getEnergyGrid();

				double low_energy = energy_grid[0];

				/* Get random energy */
				double energy = randomNumber(low_energy/10, low_energy);
				Energy pair_energy(0,energy);

				/* Get probabilities */
				double abs_prob = absorption_xs[0] / total_xs[0];
				double expected_abs = iso->getAbsorptionProb(pair_energy);

				/* Check the total cross section */
				double rel = fabs((total_xs[0] - iso->getTotalXs(pair_energy)) / total_xs[0]);
				EXPECT_NEAR(0.0,rel,eps);

				/* Check against interpolated values */
				EXPECT_NEAR(abs_prob,expected_abs,eps);

				/* Check fission if the isotope is fissile */
				if(iso->isFissile()) {
					double fis_prob = fission_xs[0] / total_xs[0];
					double expected_fis = iso->getFissionProb(pair_energy);
					/* Check against interpolated values */
					EXPECT_NEAR(fis_prob,expected_fis,eps);
				}

			}

			Log::bok() << Log::ident(1) << " - High sampling " << Log::endl;

			/* Sample high values */
			for(size_t j = 0 ; j < nrandom ; ++j) {

				/* Get energy grid */
				vector<double> energy_grid = ace_table->getEnergyGrid();

				size_t last_idx = energy_grid.size() - 1;
				double max_energy = energy_grid[last_idx];

				/* Get random energy */
				double energy = randomNumber(max_energy, 10 * max_energy);
				Energy pair_energy(0,energy);

				/* Get probabilities */
				double abs_prob = absorption_xs[last_idx] / total_xs[last_idx];
				double expected_abs = iso->getAbsorptionProb(pair_energy);

				/* Check the total cross section */
				double rel = fabs((total_xs[last_idx] - iso->getTotalXs(pair_energy)) / total_xs[last_idx]);
				EXPECT_NEAR(0.0,rel,eps);

				/* Check against interpolated values */
				EXPECT_NEAR(abs_prob,expected_abs,eps);

				/* Check fission if the isotope is fissile */
				if(iso->isFissile()) {
					double fis_prob = fission_xs[last_idx] / total_xs[last_idx];
					double expected_fis = iso->getFissionProb(pair_energy);
					/* Check against interpolated values */
					EXPECT_NEAR(fis_prob,expected_fis,eps);
				}

			}

			delete ace_table;
		}
	}

	void checkMeanFreePath(size_t begin, size_t end) {
		using namespace Helios;
		using namespace Ace;
		using namespace std;

		double eps = 5e9*numeric_limits<double>::epsilon();
		cout << "Using epsilon = " << scientific << eps << endl;

		map<string,double> isotopes_fraction;

		/* Same fraction to all isotopes */
		double fraction = 1.0 / (end - begin);
		/* Atomic density equal to 1.0 */
		double atomic = 1.0;

		/* Create material */
		vector<McObject*> ace_objects;

		/* Number of isotopes */
		for(size_t i = begin ; i < end; ++i) {
			string name = isotopes[i];
			Log::bok() << " - Add isotope " << name << " to material" <<  Log::endl;
			isotopes_fraction[name] = fraction;
			ace_objects.push_back(new AceObject(name));
		}

		ace_objects.push_back(new AceMaterialObject("test", atomic, "atom/b-cm", "atom", isotopes_fraction));

		/* Setup environment */
		environment->pushObjects(ace_objects.begin(), ace_objects.end());
		environment->setup();

		/* Number of random energies */
		size_t nrandom = 200;

		/* Get master grid */
		const MasterGrid* master_grid = environment->getModule<AceModule>()->getMasterGrid();
		/* Get energy limits for this problem */
		double min_energy = (*master_grid)[0];
		double max_energy = (*master_grid)[master_grid->size() - 1];

		/* Get isotope map of the system (we know there is only one material that contains all the isotopes) */
		map<string,AceIsotope*> isotopes = environment->getModule<AceModule>()->getIsotopeMap();

		/* Get material */
		AceMaterial* material = environment->getObject<Materials,AceMaterial>("test")[0];

		for(size_t i = 0 ; i < nrandom ; ++i) {
			/* Energy value */
			double energy_value = randomNumber(min_energy,max_energy);
			Energy energy(0,energy_value);

			/* Total cross section at this energy */
			double total_xs = 0.0;

			/* Loop over the isotopes */
			for(map<string,AceIsotope*>::iterator it = isotopes.begin() ; it != isotopes.end() ; ++it)
				total_xs += fraction * atomic * (*it).second->getTotalXs(energy);

			/* Mean free path at this energy */
			double mfp = 1.0 / total_xs;
			/* Get mean free path from the material */
			double expected_mfp = material->getMeanFreePath(energy);
			/* Relative error */
			double rel = fabs((mfp - expected_mfp) / mfp);

			EXPECT_NEAR(0.0,rel,eps);
		}
	}

	/* Environment */
	Helios::McEnvironment* environment;
};

//TEST_F(AceModuleTest, CheckProbabilities1) {
//	size_t begin = 0;
//	size_t end = (1.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities2) {
//	size_t begin = (1.0/10.0) * (double) isotopes.size();
//	size_t end = (2.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities3) {
//	size_t begin = (2.0/10.0) * (double) isotopes.size();
//	size_t end = (3.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities4) {
//	size_t begin = (3.0/10.0) * (double) isotopes.size();
//	size_t end = (4.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities5) {
//	size_t begin = (4.0/10.0) * (double) isotopes.size();
//	size_t end = (5.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities6) {
//	size_t begin = (5.0/10.0) * (double) isotopes.size();
//	size_t end = (6.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities7) {
//	size_t begin = (6.0/10.0) * (double) isotopes.size();
//	size_t end = (7.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities8) {
//	size_t begin = (7.0/10.0) * (double) isotopes.size();
//	size_t end = (8.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities9) {
//	size_t begin = (8.0/10.0) * (double) isotopes.size();
//	size_t end = (9.0/10.0) * (double) isotopes.size();
//	checkProbs(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckProbabilities10) {
//	size_t begin = (9.0/10.0) * (double) isotopes.size();
//	size_t end = isotopes.size();
//	checkProbs(begin,end);
//}

//TEST_F(AceModuleTest, CheckMeanFreePath1) {
//	size_t begin = 0;
//	size_t end = (1.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath2) {
//	size_t begin = (1.0/10.0) * (double) isotopes.size();
//	size_t end = (2.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath3) {
//	size_t begin = (2.0/10.0) * (double) isotopes.size();
//	size_t end = (3.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath4) {
//	size_t begin = (3.0/10.0) * (double) isotopes.size();
//	size_t end = (4.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath5) {
//	size_t begin = (4.0/10.0) * (double) isotopes.size();
//	size_t end = (5.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath6) {
//	size_t begin = (5.0/10.0) * (double) isotopes.size();
//	size_t end = (6.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath7) {
//	size_t begin = (6.0/10.0) * (double) isotopes.size();
//	size_t end = (7.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath8) {
//	size_t begin = (7.0/10.0) * (double) isotopes.size();
//	size_t end = (8.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath9) {
//	size_t begin = (8.0/10.0) * (double) isotopes.size();
//	size_t end = (9.0/10.0) * (double) isotopes.size();
//	checkMeanFreePath(begin,end);
//}
//
//TEST_F(AceModuleTest, CheckMeanFreePath10) {
//	size_t begin = (9.0/10.0) * (double) isotopes.size();
//	size_t end = isotopes.size();
//	checkMeanFreePath(begin,end);
//}

class AceSamplerTest : public SimpleAceTest {
protected:
	AceSamplerTest() {/* */}
	virtual ~AceSamplerTest() {/* */}

	void SetUp() {/* */}
	void TearDown() {/* */}

	void checkIsotopeSampler(size_t begin, size_t end) {
		using namespace Helios;
		using namespace Ace;
		using namespace std;

		/* Environment */
		Helios::McEnvironment* environment = new Helios::McEnvironment();

		/* Tolerance (in percentage) of the relative error for samples */
		double tolerance = 5.0;
		cout << "Using tolerance = % " << tolerance << endl;

		map<string,double> isotopes_fraction;

		/* Same fraction to all isotopes */
		double fraction = 1.0 / (end - begin);
		/* Atomic density equal to 1.0 */
		double atomic = 1.0;

		/* Create material */
		vector<McObject*> ace_objects;

		/* Number of isotopes */
		for(size_t i = begin ; i < end; ++i) {
			string name = isotopes[i];
			Log::bok() << " - Add isotope " << name << " to material" <<  Log::endl;
			isotopes_fraction[name] = fraction;
			ace_objects.push_back(new AceObject(name));
		}

		ace_objects.push_back(new AceMaterialObject("test", atomic, "atom/b-cm", "atom", isotopes_fraction));

		/* Setup environment */
		environment->pushObjects(ace_objects.begin(), ace_objects.end());
		environment->setup();

		/* Number of random energies */
		size_t nrandom = 5;
		/* Number of samples */
		size_t samples = 100000000;
		/* Base random number */
		Random random(1);

		/* Get master grid */
		const MasterGrid* master_grid = environment->getModule<AceModule>()->getMasterGrid();

		/* Get isotope map of the system (we know there is only one material that contains all the isotopes) */
		map<string,AceIsotope*> isotopes = environment->getModule<AceModule>()->getIsotopeMap();

		/* Get material */
		AceMaterial* material = environment->getObject<Materials,AceMaterial>("test")[0];

		for(size_t i = 0 ; i < nrandom ; ++i) {
			/* Get energy */
			size_t index = rand()%(master_grid->size() - 1);
			/* Get a interpolated value */
			double energy_value = random.uniform() * ((*master_grid)[index + 1] - (*master_grid)[index]) + (*master_grid)[index];
			Energy energy(0,energy_value);

			/* Total cross section at this energy */
			double total_xs = 0.0;
			/* Occurrence of each isotope */
			map<InternalIsotopeId,double> isotopes_prob;

			/* Loop over the isotopes */
			for(map<string,AceIsotope*>::iterator it = isotopes.begin() ; it != isotopes.end() ; ++it) {
				double total = fraction * atomic * (*it).second->getTotalXs(energy);
				total_xs += total;
				isotopes_prob[(*it).second->getInternalId()] = total;
			}

			/* Calculate probabilities */
			for(map<InternalIsotopeId,double>::iterator it = isotopes_prob.begin() ; it != isotopes_prob.end() ; ++it)
				(*it).second /= total_xs;

			map<InternalIsotopeId,double> isotope_samples;

			#pragma omp parallel
			{
				Random local_random(random);
			    int size=omp_get_num_threads();
			    int rank=omp_get_thread_num();
			    local_random.jump(rank*samples/size);
			    map<InternalIsotopeId,double> isotope_local_samples;

			    /* Sampling loop */
				#pragma omp for
				for(size_t j = 0 ; j < samples ; ++j) {
					const AceIsotope* isotope = dynamic_cast<const AceIsotope*>(material->getIsotope(energy,local_random));
					isotope_local_samples[isotope->getInternalId()]++;
				}

				/* Sum contributions */
				#pragma omp critical
				{
					for(map<InternalIsotopeId,double>::const_iterator it = isotope_local_samples.begin() ;
							it != isotope_local_samples.end() ; ++it) isotope_samples[(*it).first] += (*it).second;
				}
			}

			/* Collect samples and check results */
			map<InternalIsotopeId,double>::iterator it_expected = isotopes_prob.begin();
			for(map<InternalIsotopeId,double>::iterator it = isotope_samples.begin() ; it != isotope_samples.end() ; ++it) {
				(*it).second /= (double)samples;
				/* Get relative error (percentage) */
				double error = 100.0* fabs((*it_expected).second - (*it).second) / (*it_expected).second;
				EXPECT_NEAR(0.0,error,tolerance);
				++it_expected;
			}

		}

		delete environment;
	}

};

TEST_F(AceSamplerTest, IsotopeSampler) {
	size_t partition = 10;
	size_t delta = isotopes.size() / partition;
	for(size_t  i = 0 ; i < delta ; ++i)
		checkIsotopeSampler(i * partition,(i + 1) * partition);
}

#endif /* ACETESTS_HPP_ */
