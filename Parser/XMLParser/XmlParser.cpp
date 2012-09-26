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

/* Returns a string map of attributes on a node */
static inline XmlParser::AttribMap dump_attribs(TiXmlElement* pElement) {
	if ( !pElement ) return XmlParser::AttribMap(); /* No attributes */
	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	/* Map of attributes */
	XmlParser::AttribMap mapAttrib;
	while (pAttrib) {
		/* Get attribute and push it into the map */
		mapAttrib[pAttrib->Name()] = pAttrib->Value();
		pAttrib=pAttrib->Next();
	}
	/* Return map */
	return mapAttrib;
}

void XmlParser::XmlAttributes::checkAttributes(const XmlParser::AttribMap& attrib_map) {
	/* Check for required parameters */
	vector<string>::iterator it_req;
	for(it_req = required.begin() ; it_req != required.end() ; ++it_req) {
		string req_attrib = *it_req;
		if(attrib_map.find(req_attrib) == attrib_map.end()) {
			/* We should construct the keywords to locate this line on the file */
			vector<string> keywords;
			AttribMap::const_iterator it_att = attrib_map.begin();
			for(; it_att != attrib_map.end() ; ++it_att) {
				keywords.push_back((*it_att).first);
				keywords.push_back((*it_att).second);
			}
			throw KeywordParserError("Missing <" + req_attrib + "> attribute",keywords);
		}
	}

	/* If all required attributes are there, we should check the optional attributes */
	AttribMap::const_iterator it_att = attrib_map.begin();

	for(; it_att != attrib_map.end() ; ++it_att) {
		/* User attribute on input */
		string user_attrib = (*it_att).first;
		vector<string>::iterator it_exp;

		/* Check on required container */
		it_exp = find(required.begin(), required.end(), user_attrib);
		if(it_exp == required.end()) {
			/* Check on optional container */
			it_exp = find(optional.begin(), optional.end(), user_attrib);
			if(it_exp == optional.end()) {
				/* We should construct the keywords to locate this line on the file */
				vector<string> keywords;
				AttribMap::const_iterator it_att = attrib_map.begin();
				for(; it_att != attrib_map.end() ; ++it_att) {
					keywords.push_back((*it_att).first);
					keywords.push_back((*it_att).second);
				}
				throw KeywordParserError("Bad attribute keyword <" + user_attrib + ">", keywords);
			}

		}
	}
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
	std::istringstream sin(mapAttrib["coeffs"]);
	vector<double> coeffs;
	while(sin.good()) {
		double c;
		sin >> c;
		coeffs.push_back(c);
	}
	Surface::SurfaceInfo flags = sur_flags.getValue(mapAttrib);
	/* Return surface definition */
	return Geometry::SurfaceDefinition(id,type,coeffs,flags);
}

/* Initialization of values on the surface flag */
static map<string,Cell::CellInfo> initCellInfo() {
	map<string,Cell::CellInfo> values_map;
	values_map["dead"] = Cell::DEADCELL;
	values_map["negated"] = Cell::NEGATED;
	values_map["void"] = Cell::VOID;
	return values_map;
}
/* Parse cell attributes */
static Geometry::CellDefinition cellAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id", "surfaces"};
	static const string optional[4] = {"material","type","fill","universe"};
	static XmlParser::XmlAttributes cellAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 4));
	/* Cell flags values */
	XmlParser::AttributeValue<Cell::CellInfo> cell_flags("type",Cell::NONE,initCellInfo());
	XmlParser::AttributeValue<string> inp_universe("universe","0");
	XmlParser::AttributeValue<string> inp_fill("fill","0");

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	cellAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	CellId id = fromString<CellId>(mapAttrib["id"]);
	std::istringstream sin(mapAttrib["surfaces"]);
	vector<signed int> surfaces;
	while(sin.good()) {
		signed int c;
		sin >> c;
		surfaces.push_back(c);
	}
	/* Flags of the cell */
	Cell::CellInfo flags = cell_flags.getValue(mapAttrib);

	/* Get information about universes on this cell */
	UniverseId universe = fromString<UniverseId>(inp_universe.getString(mapAttrib));
	UniverseId fill = fromString<UniverseId>(inp_fill.getString(mapAttrib));

	/* Return surface definition */
	return Geometry::CellDefinition(id,surfaces,flags,universe,fill);
}

void XmlParser::geoNode(TiXmlNode* pParent) {
	vector<Geometry::SurfaceDefinition> sur_def;
	vector<Geometry::CellDefinition> cell_def;

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::TINYXML_ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "surface")
				sur_def.push_back(surfaceAttrib(pChild->ToElement()));
			else if (element_value == "cell")
				cell_def.push_back(cellAttrib(pChild->ToElement()));
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized geometry keyword <" + element_value + ">",keywords);
			}
		}
	}

	/* Add the geometries entities */
	Parser::addSurfaces(sur_def);
	Parser::addCells(cell_def);
}

void XmlParser::rootNode(TiXmlNode* pParent) const {
	int t = pParent->Type();
	TiXmlNode* pChild;

	/* Loop over "childs" */
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {

		if (t == TiXmlNode::TINYXML_ELEMENT) {
			/* Element value */
			string element_value(pParent->Value());
			map<string,NodeParser>::const_iterator it_root = root_map.find(element_value);

			/* Check node parser map */
			if(it_root != root_map.end()) {
				/* Process node */
				(*it_root).second(pParent);
			}
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized root node <" + element_value + ">",keywords);
			}

			/* Done with this node */
			return;
		}

		rootNode(pChild);
	}
}

void XmlParser::parseFile(const string& file) const {
	/* Open document */
	TiXmlDocument doc(file.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
		rootNode(&doc);
	else
		throw(ParserError("File " + file + " : " + doc.ErrorDesc()));
}

XmlParser::Parser& XmlParser::access() {
	static XmlParser parser;
	return parser;
}

XmlParser::XmlParser() {
	root_map["geometry"] = &XmlParser::geoNode;
}

} /* namespace Helios */
