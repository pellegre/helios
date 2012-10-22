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

#ifndef SOURCETEST_HPP_
#define SOURCETEST_HPP_

#include <string>

#include "../../../Common/Common.hpp"
#include "../../../Parser/ParserTypes.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"
#include "../../Transport/Source.hpp"

#include "gtest/gtest.h"

/*
 * The fixture for geometry testing class. Open and parse a geometry
 * file.
 */
class SourceTest : public ::testing::Test {

protected:
	SourceTest(const std::string& geo_file) : geo_file(geo_file) {/* */}

	virtual ~SourceTest() {/* */}

	void SetUp() {
		/* Parser (XML for now) */
		parser = new Helios::XmlParser;
		/* Environment */
		environment = new Helios::McEnvironment(parser);
		/* Parse files, to get the information to create the environment */
		environment->parseFile(InputPath::access().getPath() + "/SourceTest/" + geo_file);
		/* Setup the problem (the geometry) */
		environment->setup();
		/* Get geometry */
		geometry = environment->getModule<Helios::Geometry>();
		/* Clear the geometry objects */
		parser->clear();
	}

	void TearDown() {
		delete environment;
		delete parser;
	}
	/* Random generator */
	Helios::Random r;
	/* Name of the geometry file */
	std::string geo_file;
	/* Environment */
	Helios::McEnvironment* environment;
	/* Geometry */
	Helios::Geometry* geometry;
	/* Parser*/
	Helios::Parser* parser;
};

/* Test for "concentric" surfaces */
class CellTest : public SourceTest {
protected:
	CellTest(const std::string& geo_file, const size_t& histories) :
		SourceTest(geo_file), histories(histories) {/* */}

	virtual ~CellTest() {/* */}

	/* Random transport of a particle until a dead cell is found */
	void random(const std::string& source_file, const Helios::CellId& cellExpected) {
		/* Get source from file */
		parser->parseFile(InputPath::access().getPath() + "/SourceTest/" + source_file);
		std::vector<Helios::McObject*> sourceDefinitions = parser->getObjects(); /* Only source definitions */
		Helios::Source* source = environment->createModule<Helios::Source>(sourceDefinitions);

		for(size_t h = 0 ; h < histories ; h++) {
			/* sample particle */
			Helios::Particle sampleParticle = source->sample(r);
			/* Get coordinates */
			Helios::Coordinate position = sampleParticle.pos();
			/* Find cell */
			const Helios::Cell* cell(geometry->findCell(sampleParticle.pos()));
			/* Get cell ID */
			Helios::CellId cellId(geometry->getUserId(cell));
			/* Check */
			ASSERT_EQ(cellId,cellExpected);
		}

		delete source;
	}

	std::map<Helios::CellId,size_t> collect(const std::string& source_file, size_t samples) {
		/* Get source from file */
		parser->parseFile(InputPath::access().getPath() + "/SourceTest/" + source_file);
		std::vector<Helios::McObject*> sourceDefinitions = parser->getObjects(); /* Only source definitions */
		Helios::Source* source = environment->createModule<Helios::Source>(sourceDefinitions);

		/* Collect statistic on each cell */
		std::map<Helios::CellId,size_t> cellCount;

		for(size_t h = 0 ; h < samples ; h++) {
			/* sample particle */
			Helios::Particle sampleParticle = source->sample(r);
			/* Get coordinates */
			Helios::Coordinate position = sampleParticle.pos();
			/* Find cell */
			const Helios::Cell* cell(geometry->findCell(sampleParticle.pos()));
			/* Get cell ID */
			Helios::CellId cellPath(geometry->getPath(cell));
			/* Check */
			cellCount[cellPath]++;
		}

		delete source;
		/* Return count on each cell */
		return cellCount;
	}
	/* Number of histories */
	const size_t histories;
};

class CylinderXYCenterTest : public CellTest {
protected:
	CylinderXYCenterTest() : CellTest("geo-xy.xml",50000) {/* */};
	~CylinderXYCenterTest() {/* */}
};

TEST_F(CylinderXYCenterTest, GenCell6Particles) {random("source-xy-cell6.xml","6");}
TEST_F(CylinderXYCenterTest, GenCell7Particles) {random("source-xy-cell7.xml","7");}
TEST_F(CylinderXYCenterTest, GenCell1001Particles) {random("source-xy-cell100-1.xml","100");}
TEST_F(CylinderXYCenterTest, GenCell1002Particles) {random("source-xy-cell100-2.xml","100");}
TEST_F(CylinderXYCenterTest, GenCell1003Particles) {random("source-xy-cell100-3.xml","100");}
TEST_F(CylinderXYCenterTest, GenCell1011Particles) {random("source-xy-cell101-1.xml","101");}
TEST_F(CylinderXYCenterTest, GenCell1012Particles) {random("source-xy-cell101-2.xml","101");}
TEST_F(CylinderXYCenterTest, GenCell1013Particles) {random("source-xy-cell101-3.xml","101");}

static double roundNumber(double d)
{
  return floor(d + 0.5);
}

TEST_F(CylinderXYCenterTest, CheckCountSamplerEqual) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-3.xml",samples);
	std::map<Helios::CellId,size_t>::const_iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		int rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		ASSERT_EQ(rate,25);
	}
}

TEST_F(CylinderXYCenterTest, CheckCountSamplerWeights) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-sample-1.xml",samples);
	std::map<Helios::CellId,size_t>::iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		size_t rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		(*it).second = rate;
	}
	/* Check values */
	ASSERT_EQ(countMap["101<1"],20);
	ASSERT_EQ(countMap["101<2"],30);
	ASSERT_EQ(countMap["101<3"],10);
	ASSERT_EQ(countMap["101<4"],40);
}

TEST_F(CylinderXYCenterTest, CheckCountSourceEqual) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-sample-2.xml",samples);
	std::map<Helios::CellId,size_t>::const_iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		int rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		ASSERT_EQ(rate,25);
	}
}

TEST_F(CylinderXYCenterTest, CheckCountSourceWeights) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-sample-3.xml",samples);
	std::map<Helios::CellId,size_t>::iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		size_t rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		(*it).second = rate;
	}
	/* Check values */
	ASSERT_EQ(countMap["101<1"],20);
	ASSERT_EQ(countMap["101<2"],30);
	ASSERT_EQ(countMap["101<3"],10);
	ASSERT_EQ(countMap["101<4"],40);
}

TEST_F(CylinderXYCenterTest, CheckCountDistEqual) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-sample-4.xml",samples);
	std::map<Helios::CellId,size_t>::const_iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		int rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		ASSERT_EQ(rate,25);
	}
}

TEST_F(CylinderXYCenterTest, CheckCountDistWeights) {
	size_t samples = 1000000;
	std::map<Helios::CellId,size_t> countMap = collect("source-xy-cell101-sample-5.xml",samples);
	std::map<Helios::CellId,size_t>::iterator it = countMap.begin();
	for(;it != countMap.end() ; ++it) {
		size_t rate = roundNumber(100 * ((double)(*it).second / (double)samples));
		(*it).second = rate;
	}
	/* Check values */
	ASSERT_EQ(countMap["100<1"],20);
	ASSERT_EQ(countMap["100<2"],30);
	ASSERT_EQ(countMap["100<3"],10);
	ASSERT_EQ(countMap["100<4"],40);
}

#endif /* SOURCETEST_HPP_ */
