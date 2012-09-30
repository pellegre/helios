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
#include <map>
#include <sstream>
#include <algorithm>

#include "XmlParser.hpp"

using namespace std;

namespace Helios {

/* Initialization of values on the surface flag */
static map<string,Geometry::LatticeDefinition::LatticeInfo> initLatticeInfo() {
	map<string,Geometry::LatticeDefinition::LatticeInfo> values_map;
	values_map["rectangular"] = Geometry::LatticeDefinition::RECTANGULAR;
	return values_map;
}
/* Parse surface attributes */
static Geometry::LatticeDefinition latticeAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[4] = {"id", "dimension","pitch","universes"};
	static const string optional[1] = {"type"};
	static XmlParser::XmlAttributes latAttrib(vector<string>(required, required + 4), vector<string>(optional, optional + 1));
	/* Lattice Type */
	XmlParser::AttributeValue<Geometry::LatticeDefinition::LatticeInfo> lat_type("type",Geometry::LatticeDefinition::RECTANGULAR,initLatticeInfo());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	latAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	UniverseId id = fromString<UniverseId>(mapAttrib["id"]);
	Geometry::LatticeDefinition::LatticeInfo type = lat_type.getValue(mapAttrib);
	vector<unsigned int> dimension = getContainer<unsigned int>(mapAttrib["dimension"]);
	vector<double> width = getContainer<double>(mapAttrib["pitch"]);
	vector<UniverseId> universes = getContainer<UniverseId>(mapAttrib["universes"]);
	/* Return surface definition */
	return Geometry::LatticeDefinition(id,type,dimension,width,universes);
}

/* Initialization of values on the surface flag */
static map<string,Surface::SurfaceInfo> initSurfaceInfo() {
	map<string,Surface::SurfaceInfo> values_map;
	values_map["reflective"] = Surface::REFLECTING;
	return values_map;
}
/* Parse surface attributes */
static Geometry::SurfaceDefinition surfaceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id", "type", "coeffs"};
	static const string optional[1] = {"boundary"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 3), vector<string>(optional, optional + 1));
	/* Surface flags values */
	XmlParser::AttributeValue<Surface::SurfaceInfo> sur_flags("boundary",Surface::NONE,initSurfaceInfo());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	SurfaceId id = fromString<SurfaceId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	vector<double> coeffs = getContainer<double>(mapAttrib["coeffs"]);
	Surface::SurfaceInfo flags = sur_flags.getValue(mapAttrib);
	/* Return surface definition */
	return Geometry::SurfaceDefinition(id,type,coeffs,flags);
}

/* Initialization of values on the surface flag */
static map<string,Cell::CellInfo> initCellInfo() {
	map<string,Cell::CellInfo> values_map;
	values_map["none"] = Cell::NONE;
	values_map["dead"] = Cell::DEADCELL;
	values_map["negated"] = Cell::NEGATED;
	return values_map;
}
/* Parse cell attributes */
static Geometry::CellDefinition cellAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id", "surfaces"};
	static const string optional[5] = {"material","type","fill","universe","translation"};
	static XmlParser::XmlAttributes cellAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 5));

	/* Cell flags values */
	XmlParser::AttributeValue<string> cell_flags("type","none");
	/* Universe */
	XmlParser::AttributeValue<string> inp_universe("universe","0");
	/* Universe filling this cell */
	XmlParser::AttributeValue<string> inp_fill("fill","0");
	/* Translation */
	XmlParser::AttributeValue<string> inp_translation("translation","0 0 0");

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	cellAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	CellId id = fromString<CellId>(mapAttrib["id"]);
	vector<signed int> surfaces = getContainer<signed int>(mapAttrib["surfaces"]);

	/* Flags of the cell */
	string str_flags = reduce(cell_flags.getString(mapAttrib));
	vector<string> val_flags;
	tokenize(str_flags,val_flags,",");
	Cell::CellInfo flags = Cell::NONE;
	map<string,Cell::CellInfo> flag_map = initCellInfo();
	for(vector<string>::const_iterator it = val_flags.begin() ; it != val_flags.end() ; ++it)
		flags |= flag_map[*it];

	/* Get information about universes on this cell */
	UniverseId universe = fromString<UniverseId>(inp_universe.getString(mapAttrib));
	UniverseId fill = fromString<UniverseId>(inp_fill.getString(mapAttrib));

	/* Get the translation coefficients */
	std::istringstream sin_trans(reduce(inp_translation.getString(mapAttrib)));
	Direction trans(0,0,0);
	size_t i = 0;
	while(sin_trans.good()) {
		double c;
		sin_trans >> c;
		trans[i] = c;
		i++;
	}

	/* Return surface definition */
	return Geometry::CellDefinition(id,surfaces,flags,universe,fill,trans);
}

void XmlParser::geoNode(TiXmlNode* pParent) const {
	vector<Geometry::SurfaceDefinition> sur_def;
	vector<Geometry::CellDefinition> cell_def;
	vector<Geometry::LatticeDefinition> latt_def;

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::TINYXML_ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "surface")
				sur_def.push_back(surfaceAttrib(pChild->ToElement()));
			else if (element_value == "cell")
				cell_def.push_back(cellAttrib(pChild->ToElement()));
			else if (element_value == "lattice")
				latt_def.push_back(latticeAttrib(pChild->ToElement()));
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized geometry keyword <" + element_value + ">",keywords);
			}
		}
	}

	/* Add the geometries entities */
	setupGeometry(sur_def,cell_def,latt_def);
}

}
