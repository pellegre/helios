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
#include "../../Environment/Settings/Settings.hpp"

using namespace std;
using namespace ticpp;

namespace Helios {

/* Parse Source attributes */
static SettingsObject* settingAttrib(TiXmlElement* pElement, const string& name, const set<string>& valid_keys) {
	/* Initialize XML attribute checker */
	vector<string> required(valid_keys.begin(), valid_keys.end());
	XmlParser::XmlAttributes setAttrib(required, vector<string>());

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	setAttrib.checkAttributes(mapAttrib, name);

	/* Return surface definition */
	return new SettingsObject(name, mapAttrib);
}

void XmlParser::setNode(TiXmlNode* pParent) {
	/* Get valid settings */
	const map<UserId,set<string> >& valid_settings = Settings::getValidSettings();

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::ELEMENT) {
			string element_value(pChild->Value());
			map<UserId,set<string> >::const_iterator it = valid_settings.find(element_value);
			if (it != valid_settings.end()) {
				objects.push_back(settingAttrib(pChild->ToElement(), element_value, (*it).second));
			} else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized setting keyword <" + element_value + ">",keywords);
			}
		}
	}
}

}



