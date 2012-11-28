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

void XmlParser::XmlAttributes::checkAttributes(const XmlParser::AttribMap& attrib_map, const string& object) {
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
			throw KeywordParserError("Missing <" + req_attrib + "> attribute in " + object + " definition",keywords);
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
				throw KeywordParserError("Bad attribute keyword <" + user_attrib + "> in " + object + " definition", keywords);
			}

		}
	}
}

void XmlParser::rootNode(TiXmlNode* pParent, const string& filename) {
	int t = pParent->Type();
	TiXmlNode* pChild;

	/* Loop over children */
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {

		if (t == TiXmlNode::ELEMENT) {
			/* Element value */
			string element_value(pParent->Value());
			map<string,NodeParser>::const_iterator it_root = root_map.find(element_value);

			/* Check node parser map */
			if(it_root != root_map.end()) {
				/* File name */
				Log::msg() << left << Log::ident(1) << " - Reading node ";
				Log::color<Log::COLOR_BOLDWHITE>() << setw(9) << element_value << Log::crst;
				Log::msg()  << " from file " + filename << Log::endl;
				/* Process node */
				NodeParser parser_function = (*it_root).second;
				(this->*parser_function)(pParent);
			}
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized root node <" + element_value + "> on file " + filename,keywords);
			}

			/* Done with this node */
			return;
		}

		rootNode(pChild,filename);
	}
}

void XmlParser::parseInputFile(const string& file) {
	/* Open document */
	ticpp::Document doc(file.c_str());
	try {
		/* Load document */
		doc.LoadFile();
		rootNode(doc.GetTiXmlPointer(),file);
	} catch(ticpp::Exception& ex) {
		throw(ParserError(ex.what()));
	}
}

XmlParser::XmlParser() {
	Log::bok() << "Initializing XML Parser " << Log::endl;
	root_map["geometry"] = &XmlParser::geoNode;
	root_map["materials"] = &XmlParser::matNode;
	root_map["sources"] = &XmlParser::srcNode;
	root_map["settings"] = &XmlParser::setNode;
}

} /* namespace Helios */
