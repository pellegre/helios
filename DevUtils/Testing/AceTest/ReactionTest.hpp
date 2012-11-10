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

#include "../../../Common/Common.hpp"
#include "../../../Material/AceTable/AceModule.hpp"
#include "../../../Material/AceTable/AceMaterial.hpp"
#include "../../../Material/AceTable/AceReader/Ace.hpp"
#include "../../../Material/AceTable/AceReader/AceUtils.hpp"
#include "../../../Material/AceTable/AceReader/Conf.hpp"
#include "../../../Material/AceTable/AceReaction/ElasticScattering.hpp"
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

	Histogram<LinearBins> histo(0.01,3,75);
	Particle particle;
	particle.erg().second = 1E-8;
	particle.dir() = Direction(1.0,0.0,0.0);
	Random random(1);
	cout << particle << endl;

	/* Get elastic scattering */
	Reaction* elastic = environment->getObject<AceModule,AceIsotope>(name)[0]->getReaction(18);

	cout << *elastic << endl;

	size_t nsamples = 1000000;
	double accum_nu = 0.0;
	for(size_t i = 0 ; i < nsamples ; ++i) {

		/* Set initial data */
		particle.wgt() = 1.0;
		particle.erg().second = 1E-8;

		(*elastic)(particle,random);
		histo(particle.erg().second);

		accum_nu += particle.wgt();
	}

	histo.normalize();
	cout << histo;

	cout << "Average NU = " << accum_nu / (double) nsamples << endl;
	delete environment;
	delete elastic;
}


#endif /* REACTIONTEST_HPP_ */
