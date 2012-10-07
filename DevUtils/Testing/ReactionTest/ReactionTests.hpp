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

#ifndef REACTIONTESTS_HPP_
#define REACTIONTESTS_HPP_

#include <string>

#include "../../../Common/Common.hpp"
#include "../../../Parser/ParserTypes.hpp"
#include "../../../Log/Log.hpp"
#include "../../../Material/Sampler.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

template<class TypeReaction>
class SamplerTest : public ::testing::Test {

protected:

	SamplerTest(const std::map<TypeReaction,std::vector<double> >& sampler_map) : sampler_map(sampler_map) {/* */}
	virtual ~SamplerTest() {/* */}

	void SetUp() {
		srand(time(NULL));
		std::map<TypeReaction,std::vector<double>* > ptr_sampler_map;
		typename std::map<TypeReaction,std::vector<double> >::iterator it_rea = sampler_map.begin();
		for(; it_rea != sampler_map.end() ; ++it_rea)
			ptr_sampler_map[(*it_rea).first] = &((*it_rea).second);
		sampler = new Helios::Sampler<TypeReaction>(ptr_sampler_map);
	}

	void TearDown() {
		delete sampler;
	}

	std::map<TypeReaction,std::vector<double> > sampler_map;
	/* Sampler to test */
	Helios::Sampler<TypeReaction>* sampler;
};

template<class T>
class SamplerGenerator {

	/* Number of samples (i.e. reactions) */
	int nsamples;
	/* Number of energies */
	int nenergy;
	/* Vector of probabilities */
	std::vector<double> prob;
	/* Samples map */
	std::vector<T> samples;
public:

	SamplerGenerator(const std::vector<T>& samples, int nenergy) :
		             nsamples(samples.size()), nenergy(nenergy), prob(nenergy,1.0), samples(samples) {/* */}

	/* Get map with uniform probabilities */
	std::map<T, std::vector<double> > getUniformMap() const {
		std::map<T, std::vector<double> > m;
		typename std::vector<T>::const_iterator it_samples = samples.begin();
		for(; it_samples != samples.end() ; ++it_samples)
			m[(*it_samples)] = prob;
		return m;
	}
	/* Get zeroed map */
	std::map<T, std::vector<double> > getOddZeroedMap() const {
		std::map<T, std::vector<double> > m;
		typename std::vector<T>::const_iterator it_samples = samples.begin();
		int counter = 0;
		for(; it_samples != samples.end() ; ++it_samples) {
			if(counter%2 == 0)
				m[(*it_samples)] = prob;
			else
				m[(*it_samples)] = std::vector<double>(nenergy,0.0);
			counter++;
		}
		return m;
	}
	~SamplerGenerator() {/* */}

};

/* Test for sampling integers (uniform) */
class IntUniformSamplerTest : public SamplerTest<int> {
	int nsamples;
	int histories;
protected:
	IntUniformSamplerTest(const int& nsamples, const int& histories) :
		           SamplerTest<int>(SamplerGenerator<int>(genVector<int>(0,nsamples-1),1).getUniformMap()),
		           nsamples(nsamples), histories(histories) {/* */}
	void checkUniformSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)nsamples));
			ASSERT_EQ(sampler->sample(0,value),expect);
		}
	}
	virtual ~IntUniformSamplerTest() {/* */}
};

/* Uniform samplers */
class HeavyIntUniformSamplerTest : public IntUniformSamplerTest {
protected:
	HeavyIntUniformSamplerTest() : IntUniformSamplerTest(50000,1000000) {/* */}
	virtual ~HeavyIntUniformSamplerTest() {/* */}
};
class MediumIntUniformSamplerTest : public IntUniformSamplerTest {
protected:
	MediumIntUniformSamplerTest() : IntUniformSamplerTest(50,1000000) {/* */}
	virtual ~MediumIntUniformSamplerTest() {/* */}
};
class OneIntUniformSamplerTest : public IntUniformSamplerTest {
protected:
	OneIntUniformSamplerTest() : IntUniformSamplerTest(1,1000000) {/* */}
	virtual ~OneIntUniformSamplerTest() {/* */}
};

class IntOddZeroedSamplerTest : public SamplerTest<int> {
	int nsamples;
	int histories;
protected:
	IntOddZeroedSamplerTest(const int& nsamples, const int& histories) :
		           SamplerTest<int>(SamplerGenerator<int>(genVector<int>(0,nsamples-1),1).getOddZeroedMap()),
		           nsamples(nsamples), histories(histories) {/* */}
	void checkZeroedSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)(nsamples)));
			if(expect%2 == 1) expect--;
			ASSERT_EQ(sampler->sample(0,value),expect);
		}
	}
	virtual ~IntOddZeroedSamplerTest() {/* */}
};

/* Samplers with some zeros probabilities for some reaction */
class HeavyIntOddZeroedSamplerTest : public IntOddZeroedSamplerTest {
protected:
	HeavyIntOddZeroedSamplerTest() : IntOddZeroedSamplerTest(50000,1000000) {/* */}
	virtual ~HeavyIntOddZeroedSamplerTest() {/* */}
};
class MediumIntOddZeroedSamplerTest : public IntOddZeroedSamplerTest {
protected:
	MediumIntOddZeroedSamplerTest() : IntOddZeroedSamplerTest(50,1000000) {/* */}
	virtual ~MediumIntOddZeroedSamplerTest() {/* */}
};
class OneIntOddZeroedSamplerTest : public IntOddZeroedSamplerTest {
protected:
	OneIntOddZeroedSamplerTest() : IntOddZeroedSamplerTest(1,1000000) {/* */}
	virtual ~OneIntOddZeroedSamplerTest() {/* */}
};

TEST_F(HeavyIntUniformSamplerTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(MediumIntUniformSamplerTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(OneIntUniformSamplerTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(HeavyIntOddZeroedSamplerTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(MediumIntOddZeroedSamplerTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(OneIntOddZeroedSamplerTest, SamplingIntegers) {checkZeroedSamples();}

#endif /* REACTIONTESTS_HPP_ */
