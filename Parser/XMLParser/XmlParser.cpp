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

#include "XmlParser.hpp"

using namespace std;

namespace Helios {

static map<string,string> dump_attribs(TiXmlElement* pElement) {
	if ( !pElement ) return map<string,string>();

	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	/* Map of attributes */
	map<string,string> mapAttrib;
	while (pAttrib) {
		/* Get attribute and push it into the map */
		mapAttrib[pAttrib->Name()] = pAttrib->Value();
		pAttrib=pAttrib->Next();
	}
	/* Return map */
	return mapAttrib;
}

static Geometry::SurfaceDefinition surfaceAttrib(TiXmlElement* pElement) {
	map<string,string> mapAttrib = dump_attribs(pElement);
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
	/* Return surface definition */
	return Geometry::SurfaceDefinition(id,type,coeffs);
}

static Geometry::CellDefinition cellAttrib(TiXmlElement* pElement) {
	map<string,string> mapAttrib = dump_attribs(pElement);
	/* Get attributes */
	CellId id = fromString<CellId>(mapAttrib["id"]);
	std::istringstream sin(mapAttrib["surfaces"]);
	vector<signed int> surfaces;
	while(sin.good()) {
		signed int c;
		sin >> c;
		surfaces.push_back(c);
	}
	/* Return surface definition */
	return Geometry::CellDefinition(id,surfaces);
}

void XmlParser::getGeometryNode(TiXmlNode* pParent, vector<Geometry::SurfaceDefinition>& sur_def, vector<Geometry::CellDefinition>& cell_def) const {
	if ( !pParent ) return;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	int num;

	if (t == TiXmlNode::TINYXML_ELEMENT) {
		string element_value(pParent->Value());
		if(element_value == "geometry") {
			cout << "[@] Reading geometry " << endl;
		} else if (element_value == "surface") {
			sur_def.push_back(surfaceAttrib(pParent->ToElement()));
		} else if (element_value == "cell") {
			cell_def.push_back(cellAttrib(pParent->ToElement()));
		} else {
			cerr << "[@] Error: Unrecognized geometry keyword = " << element_value << endl;
		}

	}

	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
		getGeometryNode(pChild,sur_def,cell_def);
}

void XmlParser::getGeometryInformation(const string& geo_file,
		                    			vector<Geometry::SurfaceDefinition>& sur_def,
		                    			vector<Geometry::CellDefinition>& cell_def) const {
	/* Open document */
	TiXmlDocument doc(geo_file.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
		getGeometryNode(&doc,sur_def,cell_def);
	else
		cerr << "[@] Failed to load file : " << geo_file;
}

XmlParser::Parser& XmlParser::access() {
	static XmlParser* parser = new XmlParser;
	return (*parser);
}


} /* namespace Helios */
