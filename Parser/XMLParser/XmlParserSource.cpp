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
using namespace ticpp;

namespace Helios {

#include "IsotopeTable.cpp"

/* Box distribution */
static SourceObject* aceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[4] = {"id", "type", "isotope", "mt"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 4), vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib,"ace distribution");

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string isotope = mapAttrib["isotope"];
	convertIsotopeName(isotope,"");
	InternalId mt = fromString<InternalId>(mapAttrib["mt"]);

	/* Return surface definition */
	return new DistributionAceObject(id,isotope,mt);
}

/* Box distribution */
static SourceObject* boxAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id", "type"};
	static const string optional[3] = {"x","y","z"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 3));

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib,"box distribution");

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string type = mapAttrib["type"] + "-";
	/* Get extent on coordinates */
	vector<vector<double> > extent(3);
	/* Total coefficients */
	vector<double> coeffs;
	for(size_t i = 0 ; i < 3 ; ++i) {
		extent[i] = getContainer<double>(mapAttrib[getAxisName(i)]);
		size_t extSize = extent[i].size();
		if(extSize > 0) {
			type += getAxisName(i);
			for(size_t j = 0 ; j < extSize ; ++j)
				coeffs.push_back(extent[i][j]);
		}
	}
	/* Return surface definition */
	return new DistributionObject(type,id,coeffs);
}

/* Isotropic distribution */
static SourceObject* cylAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id", "type", "r"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 3), vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib,"cyl distribution");

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	vector<double> coeffs = getContainer<double>(mapAttrib["r"]);

	/* Return surface definition */
	return new DistributionObject(type,id,coeffs);
}

/* Isotropic distribution */
static SourceObject* isoAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id", "type"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 2), vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib,"isotropic distribution");

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	/* Return surface definition */
	return new DistributionBaseObject(type,id);
}

/* User defined distribution */
static SourceObject* customAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[3] = {"id", "type", "dist"};
	static const string optional[1] = {"weights"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 3), vector<string>(optional, optional + 1));

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib, "custom distribution");

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	vector<DistributionId> samplerIds = getContainer<DistributionId>(mapAttrib["dist"]);
	vector<double> weights = getContainer<double>(mapAttrib["weights"]);
	/* Return surface definition */
	return new DistributionCustomObject(type,id,samplerIds,weights);
}

static map<string,SourceObject(*(*)(TiXmlElement*))> initMap() {
	map<string,SourceObject(*(*)(TiXmlElement*))> m;
	m["box"] = boxAttrib;
	m["cyl"] = cylAttrib;
	m["isotropic"] = isoAttrib;
	m["custom"] = customAttrib;
	m["ace"] = aceAttrib;
	return m;
}

/* Initialization of values */
static map<string,string> initTypeDist() {
	map<string,string> values_map;
	values_map["box"] = "box";
	values_map["isotropic"] = "isotropic";
	values_map["custom"] = "custom";
	values_map["cyl-x"] = "cyl";
	values_map["cyl-y"] = "cyl";
	values_map["cyl-z"] = "cyl";
	values_map["ace"] = "ace";
	return values_map;
}

/* Initialize map of attribute parsers */
static map<string,SourceObject(*(*)(TiXmlElement*))> mapParser = initMap();

/* Parse distribution attributes */
static SourceObject* distAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id", "type"};
	static XmlParser::XmlAttributes distAttrib(vector<string>(required, required + 2), vector<string>());
	XmlParser::AttributeValue<string> typeDist("type","",initTypeDist());
	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);

	/* Get type (first check if the attribute is defined) */
	XmlParser::AttribMap::const_iterator it_att = mapAttrib.find("type");
	if(it_att == mapAttrib.end()) {
		/* Attribute is not defined, throw an exception */
		std::vector<std::string> keywords;
		it_att = mapAttrib.begin();
		for(; it_att != mapAttrib.end() ; ++it_att) {
			keywords.push_back((*it_att).first);
			keywords.push_back((*it_att).second);
		}
		throw Parser::KeywordParserError("Attribute <type> is not defined for a distribution",keywords);
	}

	string type = typeDist.getValue(mapAttrib);
	/* Return surface definition */
	return mapParser[type](pElement);
}

/* Parse Sampler attributes */
static SourceObject* samplerAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id", "pos"};
	static const string optional[4] = {"dir", "energy" , "dist", "cell"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 4));
	XmlParser::AttributeValue<string> dirAttrib("dir","1 0 0");
	XmlParser::AttributeValue<CellId> cellAttrib("cell","0");
	XmlParser::AttributeValue<double> energyAttrib("energy",1.0);

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib, "sampler");

	/* Get attributes */
	SamplerId id = fromString<DistributionId>(mapAttrib["id"]);
	Coordinate pos = getBlitzArray<double>(mapAttrib["pos"]);
	Direction dir = getBlitzArray<double>(dirAttrib.getString(mapAttrib));
	double energy = energyAttrib.getValue(mapAttrib);
	vector<DistributionId> distIds = getContainer<DistributionId>(mapAttrib["dist"]);
	CellId cell = cellAttrib.getString(mapAttrib);
	/* Return surface definition */
	return new ParticleSamplerObject(id,pos,dir,energy,distIds,cell);
}

/* Parse Source attributes */
static SourceObject* sourceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[1] = {"samplers"};
	static const string optional[2] = {"strength","weights"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 1), vector<string>(optional, optional + 2));
	XmlParser::AttributeValue<string> strengthAttrib("strength","1.0");

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib, "source");

	/* Get attributes */
	double strength = fromString<double>(strengthAttrib.getString(mapAttrib));
	vector<SamplerId> samplerIds = getContainer<SamplerId>(mapAttrib["samplers"]);
	vector<double> weights = getContainer<double>(mapAttrib["weights"]);
	/* Return surface definition */
	return new ParticleSourceObject(samplerIds,weights,strength);
}

void XmlParser::srcNode(TiXmlNode* pParent) {

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "dist")
				objects.push_back(distAttrib(pChild->ToElement()));
			else if (element_value == "sampler")
				objects.push_back(samplerAttrib(pChild->ToElement()));
			else if (element_value == "source")
				objects.push_back(sourceAttrib(pChild->ToElement()));
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized source keyword <" + element_value + ">",keywords);
			}
		}
	}
}

}
