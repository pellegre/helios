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
#include "../../../Common/Sampler.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

template<class T>
class SamplerGenerator {

	/* Number of samples (i.e. reactions) */
	size_t nsamples;
	/* Number of energies */
	size_t nenergy;
	/* Vector of probabilities */
	std::vector<double> prob;
	/* Samples map */
	std::vector<T> samples;
public:

	SamplerGenerator(const std::vector<T>& samples, size_t nenergy) :
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

template<class TypeReaction>
class SamplerPtrTest : public ::testing::Test {

protected:

	SamplerPtrTest(const std::map<TypeReaction,std::vector<double> >& sampler_map) : sampler_map(sampler_map) {/* */}
	virtual ~SamplerPtrTest() {/* */}

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

/* Test for sampling integers (uniform) */
class IntUniformSamplerPtrTest : public SamplerPtrTest<size_t> {
	size_t nsamples;
	size_t histories;
	size_t nenergies;
protected:
	IntUniformSamplerPtrTest(const size_t& nsamples, const size_t& nenergies, const size_t& histories) :
		           SamplerPtrTest<size_t>(SamplerGenerator<size_t>(genVector<size_t>(0,nsamples-1),nenergies).getUniformMap()),
		           nsamples(nsamples), histories(histories), nenergies(nenergies) {/* */}
	void checkUniformSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)nsamples));
			ASSERT_EQ(sampler->sample(rand()%nenergies,value),expect);
		}
	}
	virtual ~IntUniformSamplerPtrTest() {/* */}
};

/* Uniform samplers */
class HeavyIntUniformSamplerPtrTest : public IntUniformSamplerPtrTest {
protected:
	HeavyIntUniformSamplerPtrTest() : IntUniformSamplerPtrTest(50000,100,1000000) {/* */}
	virtual ~HeavyIntUniformSamplerPtrTest() {/* */}
};
class MaterialsIntUniformSamplerPtrTest : public IntUniformSamplerPtrTest {
protected:
	MaterialsIntUniformSamplerPtrTest() : IntUniformSamplerPtrTest(50,100,1000000) {/* */}
	virtual ~MaterialsIntUniformSamplerPtrTest() {/* */}
};
class OneIntUniformSamplerPtrTest : public IntUniformSamplerPtrTest {
protected:
	OneIntUniformSamplerPtrTest() : IntUniformSamplerPtrTest(1,100,1000000) {/* */}
	virtual ~OneIntUniformSamplerPtrTest() {/* */}
};

class IntOddZeroedSamplerPtrTest : public SamplerPtrTest<size_t> {
	size_t nsamples;
	size_t histories;
	size_t nenergies;
protected:
	IntOddZeroedSamplerPtrTest(const size_t& nsamples, const size_t& nenergies, const size_t& histories) :
		           SamplerPtrTest<size_t>(SamplerGenerator<size_t>(genVector<size_t>(0,nsamples-1),nenergies).getOddZeroedMap()),
		           nsamples(nsamples), histories(histories), nenergies(nenergies) {/* */}
	void checkZeroedSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)(nsamples)));
			if(expect%2 == 1) expect--;
			ASSERT_EQ(sampler->sample(rand()%nenergies,value),expect);
		}
	}
	virtual ~IntOddZeroedSamplerPtrTest() {/* */}
};

/* Samplers with some zeros probabilities for some reaction */
class HeavyIntOddZeroedSamplerPtrTest : public IntOddZeroedSamplerPtrTest {
protected:
	HeavyIntOddZeroedSamplerPtrTest() : IntOddZeroedSamplerPtrTest(50000,100,1000000) {/* */}
	virtual ~HeavyIntOddZeroedSamplerPtrTest() {/* */}
};
class MaterialsIntOddZeroedSamplerPtrTest : public IntOddZeroedSamplerPtrTest {
protected:
	MaterialsIntOddZeroedSamplerPtrTest() : IntOddZeroedSamplerPtrTest(50,100,1000000) {/* */}
	virtual ~MaterialsIntOddZeroedSamplerPtrTest() {/* */}
};
class OneIntOddZeroedSamplerPtrTest : public IntOddZeroedSamplerPtrTest {
protected:
	OneIntOddZeroedSamplerPtrTest() : IntOddZeroedSamplerPtrTest(1,100,1000000) {/* */}
	virtual ~OneIntOddZeroedSamplerPtrTest() {/* */}
};

TEST_F(HeavyIntUniformSamplerPtrTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(MaterialsIntUniformSamplerPtrTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(OneIntUniformSamplerPtrTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(HeavyIntOddZeroedSamplerPtrTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(MaterialsIntOddZeroedSamplerPtrTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(OneIntOddZeroedSamplerPtrTest, SamplingIntegers) {checkZeroedSamples();}

template<class TypeReaction>
class SamplerCpyTest : public ::testing::Test {

protected:

	SamplerCpyTest(const std::map<TypeReaction,std::vector<double> >& sampler_map) : sampler_map(sampler_map) {/* */}
	virtual ~SamplerCpyTest() {/* */}

	void SetUp() {
		srand(time(NULL));
		sampler = new Helios::Sampler<TypeReaction>(sampler_map);
	}

	void TearDown() {
		delete sampler;
	}

	std::map<TypeReaction,std::vector<double> > sampler_map;
	/* Sampler to test */
	Helios::Sampler<TypeReaction>* sampler;
};

/* Test for sampling integers (uniform) */
class IntUniformSamplerCpyTest : public SamplerCpyTest<size_t> {
	size_t nsamples;
	size_t histories;
	size_t nenergies;
protected:
	IntUniformSamplerCpyTest(const size_t& nsamples, const size_t& nenergies, const size_t& histories) :
		           SamplerCpyTest<size_t>(SamplerGenerator<size_t>(genVector<size_t>(0,nsamples-1),nenergies).getUniformMap()),
		           nsamples(nsamples), histories(histories),nenergies(nenergies) {/* */}
	void checkUniformSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)nsamples));
			ASSERT_EQ(sampler->sample(rand()%nenergies,value),expect);
		}
	}
	virtual ~IntUniformSamplerCpyTest() {/* */}
};

/* Uniform samplers */
class HeavyIntUniformSamplerCpyTest : public IntUniformSamplerCpyTest {
protected:
	HeavyIntUniformSamplerCpyTest() : IntUniformSamplerCpyTest(50000,100,1000000) {/* */}
	virtual ~HeavyIntUniformSamplerCpyTest() {/* */}
};
class MaterialsIntUniformSamplerCpyTest : public IntUniformSamplerCpyTest {
protected:
	MaterialsIntUniformSamplerCpyTest() : IntUniformSamplerCpyTest(50,100,1000000) {/* */}
	virtual ~MaterialsIntUniformSamplerCpyTest() {/* */}
};
class OneIntUniformSamplerCpyTest : public IntUniformSamplerCpyTest {
protected:
	OneIntUniformSamplerCpyTest() : IntUniformSamplerCpyTest(1,100,1000000) {/* */}
	virtual ~OneIntUniformSamplerCpyTest() {/* */}
};

class IntOddZeroedSamplerCpyTest : public SamplerCpyTest<size_t> {
	size_t nsamples;
	size_t histories;
	size_t nenergies;
protected:
	IntOddZeroedSamplerCpyTest(const size_t& nsamples, const size_t& nenergies, const size_t& histories) :
		           SamplerCpyTest<size_t>(SamplerGenerator<size_t>(genVector<size_t>(0,nsamples-1),nenergies).getOddZeroedMap()),
		           nsamples(nsamples), histories(histories), nenergies(nenergies) {/* */}
	void checkZeroedSamples() const {
		for(size_t h = 0 ; h < histories ; h++) {
			double value = randomNumber();
			int expect = (int) (value/(1.0/(double)(nsamples)));
			if(expect%2 == 1) expect--;
			ASSERT_EQ(sampler->sample(rand()%nenergies,value),expect);
		}
	}
	virtual ~IntOddZeroedSamplerCpyTest() {/* */}
};

/* Samplers with some zeros probabilities for some reaction */
class HeavyIntOddZeroedSamplerCpyTest : public IntOddZeroedSamplerCpyTest {
protected:
	HeavyIntOddZeroedSamplerCpyTest() : IntOddZeroedSamplerCpyTest(50000,100,1000000) {/* */}
	virtual ~HeavyIntOddZeroedSamplerCpyTest() {/* */}
};
class MaterialsIntOddZeroedSamplerCpyTest : public IntOddZeroedSamplerCpyTest {
protected:
	MaterialsIntOddZeroedSamplerCpyTest() : IntOddZeroedSamplerCpyTest(50,100,1000000) {/* */}
	virtual ~MaterialsIntOddZeroedSamplerCpyTest() {/* */}
};
class OneIntOddZeroedSamplerCpyTest : public IntOddZeroedSamplerCpyTest {
protected:
	OneIntOddZeroedSamplerCpyTest() : IntOddZeroedSamplerCpyTest(1,100,1000000) {/* */}
	virtual ~OneIntOddZeroedSamplerCpyTest() {/* */}
};

TEST_F(HeavyIntUniformSamplerCpyTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(MaterialsIntUniformSamplerCpyTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(OneIntUniformSamplerCpyTest, SamplingIntegers) {checkUniformSamples();}
TEST_F(HeavyIntOddZeroedSamplerCpyTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(MaterialsIntOddZeroedSamplerCpyTest, SamplingIntegers) {checkZeroedSamples();}
TEST_F(OneIntOddZeroedSamplerCpyTest, SamplingIntegers) {checkZeroedSamples();}

#endif /* REACTIONTESTS_HPP_ */
