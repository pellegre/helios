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

/* Parse distribution attributes */
static SourceDefinition* distAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"id", "type"};
	static const string optional[1] = {"coeffs"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 1));

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	DistributionId id = fromString<DistributionId>(mapAttrib["id"]);
	string type = mapAttrib["type"];
	vector<double> coeffs = getContainer<double>(mapAttrib["coeffs"]);
	/* Return surface definition */
	return new DistributionBase::Definition(type,id,coeffs);
}

/* Parse Sampler attributes */
static SourceDefinition* samplerAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[4] = {"id", "pos", "dir", "energy"};
	static const string optional[1] = {"dist"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 4), vector<string>(optional, optional + 1));

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	SamplerId id = fromString<DistributionId>(mapAttrib["id"]);
	Coordinate pos = getBlitzArray<double>(mapAttrib["pos"]);
	Direction dir = getBlitzArray<double>(mapAttrib["dir"]);
	vector<DistributionId> distIds = getContainer<DistributionId>(mapAttrib["dist"]);
	/* Return surface definition */
	return new ParticleSampler::Definition(id,pos,dir,distIds);
}

/* Parse Source attributes */
static SourceDefinition* sourceAttrib(TiXmlElement* pElement) {
	/* Initialize XML attribute checker */
	static const string required[2] = {"strength", "samplers"};
	static const string optional[1] = {"weights"};
	static XmlParser::XmlAttributes surAttrib(vector<string>(required, required + 2), vector<string>(optional, optional + 1));

	XmlParser::AttribMap mapAttrib = dump_attribs(pElement);
	/* Check user input */
	surAttrib.checkAttributes(mapAttrib);

	/* Get attributes */
	double strength = fromString<double>(mapAttrib["strength"]);
	vector<SamplerId> samplerIds = getContainer<SamplerId>(mapAttrib["samplers"]);
	vector<double> weights = getContainer<double>(mapAttrib["weights"]);
	/* Return surface definition */
	return new ParticleSource::Definition(samplerIds,weights,strength);
}

void XmlParser::srcNode(TiXmlNode* pParent) {

	TiXmlNode* pChild;
	for (pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		int t = pChild->Type();
		if (t == TiXmlNode::TINYXML_ELEMENT) {
			string element_value(pChild->Value());
			if (element_value == "dist")
				sourceDefinition.push_back(distAttrib(pChild->ToElement()));
			else if (element_value == "sampler")
				sourceDefinition.push_back(samplerAttrib(pChild->ToElement()));
			else if (element_value == "source")
				sourceDefinition.push_back(sourceAttrib(pChild->ToElement()));
			else {
				vector<string> keywords;
				keywords.push_back(element_value);
				throw KeywordParserError("Unrecognized source keyword <" + element_value + ">",keywords);
			}
		}
	}
}

}
