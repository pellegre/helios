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

#ifndef GEOMETRYTESTS_HPP_
#define GEOMETRYTESTS_HPP_

#include <string>

#include "../../../Common/Common.hpp"
#include "../../../Parser/ParserTypes.hpp"
#include "../../../Log/Log.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

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
		/* Parser (XML for now) */
		parser = new Helios::XmlParser;
		/* Parse the file */
		parser->parseFile(InputPath::access().getPath() + "/GeometryTest/" + filename);
		/* Geometry */
		std::vector<Helios::GeometricDefinition*> geometryDefinitions = parser->getGeometry();
		geometry = new Helios::Geometry(geometryDefinitions);
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
	ConcentricTest(const std::string& filename,const Helios::Coordinate& start_position, const size_t& histories) :
					GeometryTest(filename), start_position(start_position), histories(histories) {/* */}

	virtual ~ConcentricTest() {/* */}

	/* Straight transport of the particle until a dead cell is found*/
	void straight(const std::vector<Helios::CellId>& expected_cells, const std::vector<Helios::SurfaceId>& expected_surfaces) const {
		for(size_t h = 0 ; h < histories ; h++) {
			std::vector<Helios::CellId> cells;
			std::vector<Helios::SurfaceId> surfaces;
			Helios::Direction start_direction = randomDirection();

			transport(*geometry,start_position,start_direction,cells,surfaces);

			if(expected_cells.size() > 0) {
				EXPECT_EQ(expected_cells.size(),cells.size());
				if(expected_cells.size() != cells.size()) {
					std::cout << "Start direction = " << start_direction << std::endl;
					/* Print the cells to see what is going on */
					for (size_t i = 0; i < cells.size(); ++i)
						std::cout << "Cell[" << i << "] = " << cells[i] << std::endl;
					ASSERT_EQ(1,0);
				}
			}

			if(expected_surfaces.size() > 0) {
				EXPECT_EQ(expected_surfaces.size(),surfaces.size());
				if(expected_surfaces.size() != surfaces.size()) {
					std::cout << "Start direction = " << start_direction << std::endl;
					/* Print the surfaces to see what is going on */
					for (size_t i = 0; i < surfaces.size(); ++i)
						std::cout << "Surface[" << i << "] = " << surfaces[i] << std::endl;
					ASSERT_EQ(1,0);
				}
			}

			for (size_t i = 0; i < expected_cells.size(); ++i)
				ASSERT_EQ(expected_cells[i],cells[i]);

			for (size_t i = 0; i < expected_surfaces.size(); ++i)
				ASSERT_EQ(expected_surfaces[i],surfaces[i]);
		}
	}

	/* Random transport of a particle until a dead cell is found */
	void random() {
		for(size_t h = 0 ; h < histories ; h++) {
			double max_eval = randomTransport(*geometry,start_position);
			EXPECT_NEAR(0.0,max_eval,5e6*std::numeric_limits<double>::epsilon());
		}
	}

	/* Initial position */
	const Helios::Coordinate start_position;
	/* Number of histories */
	const size_t histories;
};

/* Cylinder in x-y plane */
class CylinderXYConcentricTest : public ConcentricTest {
protected:
	CylinderXYConcentricTest() : ConcentricTest("cyl-xy.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTest() {/* */}
};
/* Cylinder in y-z plane */
class CylinderYZConcentricTest : public ConcentricTest {
protected:
	CylinderYZConcentricTest() : ConcentricTest("cyl-yz.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTest() {/* */}
};
/* Cylinder in x-z plane */
class CylinderXZConcentricTest : public ConcentricTest {
protected:
	CylinderXZConcentricTest() : ConcentricTest("cyl-xz.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTest() {/* */}
};

/* Cylinder in x-y plane not centered in origin */
class CylinderXYConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderXYConcentricTranslatedTest() : ConcentricTest("cyl-xy-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTranslatedTest() {/* */}
};
/* Cylinder in y-z plane not centered in origin */
class CylinderYZConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderYZConcentricTranslatedTest() : ConcentricTest("cyl-yz-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTranslatedTest() {/* */}
};
/* Cylinder in x-z plane not centered in origin */
class CylinderXZConcentricTranslatedTest : public ConcentricTest {
protected:
	CylinderXZConcentricTranslatedTest() : ConcentricTest("cyl-xz-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTranslatedTest() {/* */}
};

/* Cylinder in x-y plane using universes */
class CylinderXYConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderXYConcentricUniverseTest() : ConcentricTest("cyl-xy-univ.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricUniverseTest() {/* */}
};
/* Cylinder in y-z plane using universes */
class CylinderYZConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderYZConcentricUniverseTest() : ConcentricTest("cyl-yz-univ.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricUniverseTest() {/* */}
};
/* Cylinder in x-z plane using universes */
class CylinderXZConcentricUniverseTest : public ConcentricTest {
protected:
	CylinderXZConcentricUniverseTest() : ConcentricTest("cyl-xz-univ.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricUniverseTest() {/* */}
};

/* Cylinder in x-y plane not centered in origin using universes */
class CylinderXYConcentricTransUniverseTest : public ConcentricTest {
protected:
	CylinderXYConcentricTransUniverseTest() : ConcentricTest("cyl-xy-univ-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTransUniverseTest() {/* */}
};
/* Cylinder in x-y plane not centered in origin using universes */
class CylinderYZConcentricTransUniverseTest : public ConcentricTest {
protected:
	CylinderYZConcentricTransUniverseTest() : ConcentricTest("cyl-yz-univ-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTransUniverseTest() {/* */}
};
/* Cylinder in x-y plane not centered in origin using universes */
class CylinderXZConcentricTransUniverseTest : public ConcentricTest {
protected:
	CylinderXZConcentricTransUniverseTest() : ConcentricTest("cyl-xz-univ-trans.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTransUniverseTest() {/* */}
};

/* Cylinder in x-y plane using universes with translations */
class CylinderXYConcentricTranslaUniverseTest : public ConcentricTest {
protected:
	CylinderXYConcentricTranslaUniverseTest() : ConcentricTest("cyl-xy-univ-transla.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXYConcentricTranslaUniverseTest() {/* */}
};
/* Cylinder in y-z plane using universes with translations */
class CylinderYZConcentricTranslaUniverseTest : public ConcentricTest {
protected:
	CylinderYZConcentricTranslaUniverseTest() : ConcentricTest("cyl-yz-univ-transla.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderYZConcentricTranslaUniverseTest() {/* */}
};
/* Cylinder in x-z plane using universes with translations */
class CylinderXZConcentricTranslaUniverseTest : public ConcentricTest {
protected:
	CylinderXZConcentricTranslaUniverseTest() : ConcentricTest("cyl-xz-univ-transla.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~CylinderXZConcentricTranslaUniverseTest() {/* */}
};

/* Simple x-y lattice with *a lot* of pins. To checkout the random transport on heavy lattices */
class LatticeXYConcentricTest : public ConcentricTest {
protected:
	LatticeXYConcentricTest() : ConcentricTest("cyl-xy-latt.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~LatticeXYConcentricTest() {/* */}
};

/* Simple x-y lattice with *a lot* of pins. To checkout the random transport on heavy lattices */
class HugeLatticeXYConcentricTest : public ConcentricTest {
protected:
	HugeLatticeXYConcentricTest() : ConcentricTest("cyl-xy-latt-huge.xml",Helios::Coordinate(0,0,0),50000) {/* */};
	~HugeLatticeXYConcentricTest() {/* */}
};

TEST_F(CylinderXYConcentricTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderYZConcentricTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderXZConcentricTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderXYConcentricTranslatedTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderYZConcentricTranslatedTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderXZConcentricTranslatedTest, StraightTransport) {straight(genVector<Helios::CellId>(1,5),genVector<Helios::SurfaceId>(1,4));}
TEST_F(CylinderXYConcentricUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderYZConcentricUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderXZConcentricUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderXYConcentricTransUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderYZConcentricTransUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderXZConcentricTransUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderXYConcentricTranslaUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderYZConcentricTranslaUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}
TEST_F(CylinderXZConcentricTranslaUniverseTest, StraightTransport) {straight(genVector<Helios::CellId>(1,10),genVector<Helios::SurfaceId>(1,9));}

TEST_F(CylinderXYConcentricTest, RandomTransport) {random();}
TEST_F(CylinderYZConcentricTest, RandomTransport) {random();}
TEST_F(CylinderXZConcentricTest, RandomTransport) {random();}
TEST_F(CylinderXYConcentricTranslatedTest, RandomTransport) {random();}
TEST_F(CylinderYZConcentricTranslatedTest, RandomTransport) {random();}
TEST_F(CylinderXZConcentricTranslatedTest, RandomTransport) {random();}
TEST_F(CylinderXYConcentricUniverseTest, RandomTransport) {random();}
TEST_F(CylinderYZConcentricUniverseTest, RandomTransport) {random();}
TEST_F(CylinderXZConcentricUniverseTest, RandomTransport) {random();}
TEST_F(CylinderXYConcentricTransUniverseTest, RandomTransport) {random();}
TEST_F(CylinderYZConcentricTransUniverseTest, RandomTransport) {random();}
TEST_F(CylinderXZConcentricTransUniverseTest, RandomTransport) {random();}
TEST_F(CylinderXYConcentricTranslaUniverseTest, RandomTransport) {random();}
TEST_F(CylinderYZConcentricTranslaUniverseTest, RandomTransport) {random();}
TEST_F(CylinderXZConcentricTranslaUniverseTest, RandomTransport) {random();}

TEST_F(LatticeXYConcentricTest, RandomTransport) {random();}
TEST_F(HugeLatticeXYConcentricTest, RandomTransport) {random();}

#endif /* GEOMETRYTESTS_HPP_ */
