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

static map<string,string> initIsotopeName() {
	map<string,string> m;
	m["H"] = "1";
	m["He"] = "2";
	m["Li"] = "3";
	m["Be"] = "4";
	m["B"] = "5";
	m["C"] = "6";
	m["N"] = "7";
	m["O"] = "8";
	m["F"] = "9";
	m["Ne"] = "10";
	m["Na"] = "11";
	m["Mg"] = "12";
	m["Al"] = "13";
	m["Si"] = "14";
	m["P"] = "15";
	m["S"] = "16";
	m["Cl"] = "17";
	m["Ar"] = "18";
	m["K"] = "19";
	m["Ca"] = "20";
	m["Sc"] = "21";
	m["Ti"] = "22";
	m["V"] = "23";
	m["Cr"] = "24";
	m["Mn"] = "25";
	m["Fe"] = "26";
	m["Co"] = "27";
	m["Ni"] = "28";
	m["Cu"] = "29";
	m["Zn"] = "30";
	m["Ga"] = "31";
	m["Ge"] = "32";
	m["As"] = "33";
	m["Se"] = "34";
	m["Br"] = "35";
	m["Kr"] = "36";
	m["Rb"] = "37";
	m["Sr"] = "38";
	m["Y"] = "39";
	m["Zr"] = "40";
	m["Nb"] = "41";
	m["Mo"] = "42";
	m["Tc"] = "43";
	m["Ru"] = "44";
	m["Rh"] = "45";
	m["Pd"] = "46";
	m["Ag"] = "47";
	m["Cd"] = "48";
	m["In"] = "49";
	m["Sn"] = "50";
	m["Sb"] = "51";
	m["Te"] = "52";
	m["I"] = "53";
	m["Xe"] = "54";
	m["Cs"] = "55";
	m["Ba"] = "56";
	m["La"] = "57";
	m["Ce"] = "58";
	m["Pr"] = "59";
	m["Nd"] = "60";
	m["Pm"] = "61";
	m["Sm"] = "62";
	m["Eu"] = "63";
	m["Gd"] = "64";
	m["Tb"] = "65";
	m["Dy"] = "66";
	m["Ho"] = "67";
	m["Er"] = "68";
	m["Tm"] = "69";
	m["Yb"] = "70";
	m["Lu"] = "71";
	m["Hf"] = "72";
	m["Ta"] = "73";
	m["W"] = "74";
	m["Re"] = "75";
	m["Os"] = "76";
	m["Ir"] = "77";
	m["Pt"] = "78";
	m["Au"] = "79";
	m["Hg"] = "80";
	m["Tl"] = "81";
	m["Pb"] = "82";
	m["Bi"] = "83";
	m["Po"] = "84";
	m["At"] = "85";
	m["Rn"] = "86";
	m["Fr"] = "87";
	m["Ra"] = "88";
	m["Ac"] = "89";
	m["Th"] = "90";
	m["Pa"] = "91";
	m["U"] = "92";
	m["Np"] = "93";
	m["Pu"] = "94";
	m["Am"] = "95";
	m["Cm"] = "96";
	m["Bk"] = "97";
	m["Cf"] = "98";
	m["Es"] = "99";
	m["Fm"] = "100";
	m["Md"] = "101";
	m["No"] = "102";
	m["Lr"] = "103";
	m["Rf"] = "104";
	m["Db"] = "105";
	m["Sg"] = "106";
	m["Bh"] = "107";
	m["Hs"] = "108";
	m["Mt"] = "109";
	m["Ds"] = "110";
	return m;
}

/* Map isotope human readable name to atomic mass */
static const map<string,string> isotope_name = initIsotopeName();

static void convertZaid(string& zaid) {
	using namespace boost;
	/* Check if it is a string or a ZAID identifier */
	if(zaid.find("-") != string::npos) {
		char_separator<char> sep("- ");
		/* Get user ID */
		tokenizer<char_separator<char> > tok(zaid,sep);
		/* Get iterator */
		tokenizer<char_separator<char> >::iterator it = tok.begin();
		/* Get element */
		string element = (*it++);
		/* Get isotope */
		string isotope = (*it);
		/* Find element on the table */
		map<string,string>::const_iterator it_element = isotope_name.find(element);
		if(it_element != isotope_name.end()) {
			/* Put zeroes on the isotope part */
			for(size_t i = 0 ; i < isotope.size() - 3 ; ++i)
				isotope = "0" + isotope;
			zaid = (*it_element).second + isotope;
		}
	}
}

static void convertIsotopeName(string& isotope, const string& dataset) {
	using namespace boost;
	/* Check if there is a DataSet related to the isotope */
	if(isotope.find(".") != string::npos) {
		char_separator<char> sep(". ");
		/* Get user ID */
		tokenizer<char_separator<char> > tok(isotope,sep);
		/* Get iterator */
		tokenizer<char_separator<char> >::iterator it = tok.begin();
		/* Get ZAID */
		string zaid = (*it++);
		convertZaid(zaid);
		/* Get DataSet */
		string ds = (*it);
		/* Put back isotope name */
		isotope = zaid + "." + ds;
	} else {
		/* Get ZAID */
		string zaid = isotope;
		convertZaid(zaid);
		/* Put back isotope name */
		isotope = zaid + "." + dataset;
	}
}

/* Parse cell attributes */
static vector<McObject*> aceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[4] = {"id","density","units","fraction"};
	static const string optional[1] = {"dataset"};
	static XmlParser::XmlAttributes matAttrib(vector<string>(required, required + 4), vector<string>(optional, optional + 1));

	/* DataSet information */
	XmlParser::AttributeValue<string> inp_dataset("dataset","");

	/* Check flags */
	XmlParser::AttributeValue<string> units_flag("units","",initUnits());
	XmlParser::AttributeValue<string> fraction_flag("fraction","",initFraction());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	matAttrib.checkAttributes(mapAttrib, "material");

	/* Get attributes */
	MaterialId id = fromString<MaterialId>(mapAttrib["id"]);
	double density = fromString<double>(mapAttrib["density"]);
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
		pair<string,double> pair_value = isoAttrib(pChild);
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



