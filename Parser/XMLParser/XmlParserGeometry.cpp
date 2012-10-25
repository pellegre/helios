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

/* Parse surface attributes */
static FeatureObject* latticeAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[5] = {"id","type","dimension","pitch","universes"};
	static XmlParser::XmlAttributes latAttrib(vector<string>(required, required + 5), vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	latAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	UniverseId id = fromString<UniverseId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	vector<int> dimension = getContainer<int>(mapAttrib["dimension"]);
	vector<double> width = getContainer<double>(mapAttrib["pitch"]);
	vector<UniverseId> universes = getContainer<UniverseId>(mapAttrib["universes"]);
	/* Return surface definition */
	return new LatticeObject(id,type,dimension,width,universes);
}

/* Initialization of values on the surface flag */
static map<string,Surface::SurfaceInfo> initSurfaceInfo() {
	map<string,Surface::SurfaceInfo> values_map;
	values_map["reflective"] = Surface::REFLECTING;
	values_map["vacuum"] = Surface::VACUUM;
	return values_map;
}
/* Parse surface attributes */
static SurfaceObject* surfaceAttrib(TiXmlElement* pElement) {
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
	return new SurfaceObject(id,type,coeffs,flags);
}

/* Initialization of values on the surface flag */
static map<string,Cell::CellInfo> initCellInfo() {
	map<string,Cell::CellInfo> values_map;
	values_map["none"] = Cell::NONE;
	values_map["dead"] = Cell::DEADCELL;
	return values_map;
}
/* Parse cell attributes */
static CellObject* cellAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id"};
	static const string optional[6] = {"material","type","fill","universe","translation","surfaces"};
	static XmlParser::XmlAttributes cellAttrib(vector<string>(required, required + 1), vector<string>(optional, optional + 6));

	/* Cell flags values */
	XmlParser::AttributeValue<Cell::CellInfo> cell_flags("type",Cell::NONE,initCellInfo());
	/* Universe */
	XmlParser::AttributeValue<string> inp_universe("universe",Universe::BASE);
	/* Universe filling this cell */
	XmlParser::AttributeValue<string> inp_fill("fill","0");
	/* Translation */
	XmlParser::AttributeValue<string> inp_translation("translation","0 0 0");
	/* Material */
	XmlParser::AttributeValue<string> inp_material("material",Material::NONE);

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	cellAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	CellId id = fromString<CellId>(mapAttrib["id"]);
	string surfaces_expression = mapAttrib["surfaces"];

	/* Flags of the cell */
	Cell::CellInfo flags = cell_flags.getValue(mapAttrib);

	/* Get information about universes on this cell */
	UniverseId universe = fromString<UniverseId>(inp_universe.getString(mapAttrib));
	UniverseId fill = fromString<UniverseId>(inp_fill.getString(mapAttrib));
	MaterialId matid = fromString<MaterialId>(inp_material.getString(mapAttrib));

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
	return new CellObject(id,surfaces_expression,flags,universe,fill,matid,trans);
}

void XmlParser::geoNode(TiXmlNode* pParent) {

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::TINYXML_ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "surface")
				objects.push_back(surfaceAttrib(pChild->ToElement()));
			else if (element_value == "cell")
				objects.push_back(cellAttrib(pChild->ToElement()));
			else if (element_value == "lattice")
				objects.push_back(latticeAttrib(pChild->ToElement()));
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized geometry keyword <" + element_value + ">",keywords);
			}
		}
	}

}

}
