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

#ifndef TESTS_HPP_
#define TESTS_HPP_

#include <string>

#include "../../Common.hpp"
#include "../../Parser/ParserTypes.hpp"
#include "../../Log/Log.hpp"
#include "../Utils.hpp"

#include "gtest/gtest.h"

class InputPath {
	static InputPath inputpath;
	std::string path;
	InputPath() : path("./") {/* */}
public:
	static InputPath& access() {return inputpath;}
	std::string getPath() const {return path;}
	void setPath(const std::string& newpath) {path = newpath;}
	~InputPath() {/* */}
};

static inline std::vector<unsigned int> genVector(size_t min, size_t max) {
	std::vector<unsigned int> v;
	for(size_t i = min ; i <= max ; i++) {
		v.push_back(i);
	}
	return v;
}

/*
 * The fixture for geometry testing class. Open and parse a geometry
 * file.
 */
class GeometryTest : public ::testing::Test {

protected:
	GeometryTest(const std::string& filename) : filename(filename) {/* */}

	virtual ~GeometryTest() {/* */}

	void SetUp() {
		srand(time(NULL));
		/* Geometry */
		geometry = new Helios::Geometry;
		/* Parser (XML for now) */
		parser = new Helios::XmlParser(*geometry);
		/* Parse the file */
		parser->parseFile(InputPath::access().getPath() + filename);
	}

	void TearDown() {
		delete geometry;
		delete parser;
	}

	/* Name of the file */
	std::string filename;
	/* Geometry */
	Helios::Geometry* geometry;
	/* Parser*/
	Helios::Parser* parser;
};

/* Test for "concentric" surfaces */
class ConcentricTest : public GeometryTest {
protected:
	ConcentricTest(const std::string& filename,const std::vector<Helios::CellId>& cells, const std::vector<Helios::SurfaceId>& surfaces,
				   const Helios::Coordinate& start_position, const size_t& histories) :
	GeometryTest(filename), expected_cells(cells),expected_surfaces(surfaces),start_position(start_position), histories(histories) {/* */}

	virtual ~ConcentricTest() {/* */}

	/* Random transport of the particle */
	void randomTransport() const {
		for(size_t h = 0 ; h < histories ; h++) {
			std::vector<Helios::CellId> cells;
			std::vector<Helios::SurfaceId> surfaces;
			Helios::Direction start_direction = randomDirection();

			transport(*geometry,start_position,start_direction,cells,surfaces);

			ASSERT_EQ(expected_cells.size(),cells.size());
			ASSERT_EQ(expected_surfaces.size(),surfaces.size());

			for (size_t i = 0; i < expected_cells.size(); ++i)
			  EXPECT_EQ(expected_cells[i],cells[i]);

			for (size_t i = 0; i < expected_surfaces.size(); ++i)
			  EXPECT_EQ(expected_surfaces[i],surfaces[i]);
		}
	}

	/* Expected user cells and surfaces */
	const std::vector<Helios::CellId> expected_cells;
	const std::vector<Helios::SurfaceId> expected_surfaces;
	/* Initial position */
	const Helios::Coordinate start_position;
	/* Number of histories */
	const size_t histories;
};

/* Cylinder in x-y plane */
class CylinderXYConcentricTest : public ConcentricTest {
protected:
	CylinderXYConcentricTest() : ConcentricTest("cyl-xy.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTest() {/* */}
};
/* Cylinder in y-z plane */
class CylinderYZConcentricTest : public ConcentricTest {
protected:
	CylinderYZConcentricTest() : ConcentricTest("cyl-yz.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTest() {/* */}
};
/* Cylinder in x-z plane */
class CylinderXZConcentricTest : public ConcentricTest {
protected:
	CylinderXZConcentricTest() : ConcentricTest("cyl-xz.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTest() {/* */}
};

/* Cylinder in x-y plane with arbitrary translations */
class CylinderXYConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderXYConcentricTranslatedTest() : ConcentricTest("cyl-xy-trans.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTranslatedTest() {/* */}
};
/* Cylinder in y-z plane with arbitrary translations */
class CylinderYZConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderYZConcentricTranslatedTest() : ConcentricTest("cyl-yz-trans.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTranslatedTest() {/* */}
};
/* Cylinder in x-z plane with arbitrary translations */
class CylinderXZConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderXZConcentricTranslatedTest() : ConcentricTest("cyl-xz-trans.xml",genVector(1,5),genVector(1,4),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTranslatedTest() {/* */}
};

/* Cylinder in x-y plane with arbitrary translations */
class CylinderXYConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderXYConcentricUniverseTest() : ConcentricTest("cyl-xy-univ.xml",genVector(1,10),genVector(1,9),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricUniverseTest() {/* */}
};
/* Cylinder in y-z plane with arbitrary translations */
class CylinderYZConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderYZConcentricUniverseTest() : ConcentricTest("cyl-yz-univ.xml",genVector(1,10),genVector(1,9),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricUniverseTest() {/* */}
};
/* Cylinder in x-z plane with arbitrary translations */
class CylinderXZConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderXZConcentricUniverseTest() : ConcentricTest("cyl-xz-univ.xml",genVector(1,10),genVector(1,9),Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricUniverseTest() {/* */}
};

TEST_F(CylinderXYConcentricTest, StraightTransport) {randomTransport();}
TEST_F(CylinderYZConcentricTest, StraightTransport) {randomTransport();}
TEST_F(CylinderXZConcentricTest, StraightTransport) {randomTransport();}
TEST_F(CylinderXYConcentricTranslatedTest, StraightTransport) {randomTransport();}
TEST_F(CylinderYZConcentricTranslatedTest, StraightTransport) {randomTransport();}
TEST_F(CylinderXZConcentricTranslatedTest, StraightTransport) {randomTransport();}
TEST_F(CylinderXYConcentricUniverseTest, StraightTransport) {randomTransport();}
TEST_F(CylinderYZConcentricUniverseTest, StraightTransport) {randomTransport();}
TEST_F(CylinderXZConcentricUniverseTest, StraightTransport) {randomTransport();}

#endif /* TESTS_HPP_ */
