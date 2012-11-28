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
#include <boost/tokenizer.hpp>

#include "XmlParser.hpp"
#include "../../Material/MacroXs/MacroXs.hpp"
#include "../../Material/AceTable/AceMaterial.hpp"
#include "../../Material/AceTable/AceModule.hpp"

using namespace std;

namespace Helios {

/* Parse cell attributes */
static McObject* macroAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[6] = {"id","sigma_a","sigma_f","nu_sigma_f","chi","sigma_s"};
	static XmlParser::XmlAttributes matAttrib(vector<string>(required, required + 6), vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	matAttrib.checkAttributes(mapAttrib, "macro-xs");
	/* Constants */
	std::map<std::string,std::vector<double> > constant;

	for(XmlParser::AttribMap::const_iterator it = mapAttrib.begin() ; it != mapAttrib.end() ; ++it) {
		string attrib_name = (*it).first;
		if(attrib_name != "id")
			constant[attrib_name] = getContainer<double>((*it).second);
	}
	/* Get attributes */
	MaterialId mat_id = fromString<MaterialId>(mapAttrib["id"]);

	/* Return surface definition */
	return new MacroXsObject(mat_id,constant);
}

pair<string,double> isoAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"name","fraction"};
	static XmlParser::XmlAttributes matAttrib(vector<string>(required, required + 2), vector<string>());

	/* Check user input */
	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	matAttrib.checkAttributes(mapAttrib, "isotope");

	string isotope = fromString<string>(mapAttrib["name"]);
	/* Remove spaces on the name */
	double fraction = fromString<double>(mapAttrib["fraction"]);
	return pair<string,double>(isotope,fraction);
}

static map<string,string> initUnits() {
	map<string,string> values_map;
	values_map["g/cm3"] = "g/cm3";
	values_map["atom/b-cm"] = "atom/b-cm";
	return values_map;
}

static map<string,string> initFraction() {
	map<string,string> values_map;
	values_map["atom"] = "atom";
	values_map["weight"] = "weight";
	return values_map;
}

/* Include data os the isotope table */
#include "IsotopeTable.cpp"

/* Parse cell attributes */
static vector<McObject*> aceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id","density"};
	static const string optional[3] = {"dataset","units","fraction"};
	static XmlParser::XmlAttributes matAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 3));

	/* DataSet information */
	XmlParser::AttributeValue<string> inp_dataset("dataset","");

	/* Check flags */
	XmlParser::AttributeValue<string> units_flag("units","atom/b-cm",initUnits());
	XmlParser::AttributeValue<string> fraction_flag("fraction","atom",initFraction());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	matAttrib.checkAttributes(mapAttrib, "material");

	/* Get attributes */
	MaterialId id = fromString<MaterialId>(mapAttrib["id"]);
	double density = 0.0;
	string density_value = mapAttrib["density"];
	if(density_value != "sum")
		density = fromString<double>(mapAttrib["density"]);
	string units = units_flag.getValue(mapAttrib);
	string fraction = fraction_flag.getValue(mapAttrib);
	string dataset = inp_dataset.getString(mapAttrib);

	/* Push all the ACE objects (including the isotopes) */
	vector<McObject*> ace_objects;

	/* Get isotopes */
	TiXmlElement* pChild;
	map<string,double> isotopes;
	for (pChild = pElement->FirstChildElement(); pChild != 0; pChild = pChild->NextSiblingElement()) {
		string element_value(pChild->Value());

		/* Check name of the isotope */
		if(element_value != "isotope") {
			/* Duplicated name of isotope */
			std::vector<std::string> keywords;
			XmlParser::AttribMap::const_iterator it_att = mapAttrib.begin();
			for(; it_att != mapAttrib.end() ; ++it_att) {
				keywords.push_back((*it_att).first);
				keywords.push_back((*it_att).second);
			}
			throw Parser::KeywordParserError("Unrecognized node " + element_value + " inside material definition " + id,keywords);
		}

		pair<string,double> pair_value = isoAttrib(pChild);
		/* Sum density */
		if(density_value == "sum")
			density += pair_value.second;
		/* Map human readable name to the isotope ZAID */
		convertIsotopeName(pair_value.first, dataset);
		/* Get isotope name */
		string isotope = pair_value.first;
		/* Check if the user is not duplicating the isotope name */
		map<string,double>::iterator it = isotopes.find(isotope);
		if(it == isotopes.end()) {
			/* Push isotope */
			isotopes.insert(pair_value);
			ace_objects.push_back(new AceObject(pair_value.first));
		} else {
			/* Duplicated name of isotope */
			std::vector<std::string> keywords;
			XmlParser::AttribMap::const_iterator it_att = mapAttrib.begin();
			for(; it_att != mapAttrib.end() ; ++it_att) {
				keywords.push_back((*it_att).first);
				keywords.push_back((*it_att).second);
			}
			throw Parser::KeywordParserError("Duplicated isotope with name " + isotope,keywords);
		}
	}
	/* Return surface definition */
	ace_objects.push_back(new AceMaterialObject(id, density, units, fraction, isotopes));
	return ace_objects;
}

void XmlParser::matNode(TiXmlNode* pParent) {

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "macro-xs")
				objects.push_back(macroAttrib(pChild->ToElement()));
			else if (element_value == "material") {
				vector<McObject*> ace_objects = aceAttrib(pChild->ToElement());
				objects.insert(objects.end(), ace_objects.begin(), ace_objects.end());
			} else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized material keyword <" + element_value + ">",keywords);
			}
		}
	}

}

}



