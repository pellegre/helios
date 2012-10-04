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

#include <cstdlib>

#include "Parser.hpp"

using namespace std;

namespace Helios {

Geometry* Parser::getGeometry() {
	/* Geometry */
	Geometry* geometry = new Geometry;

	try {
		/* Add surface into the geometry */
		geometry->setupGeometry(surfaceDefinition,cellDefinition,featureDefinition);
	} catch (std::exception& exception) {
		/* Catch exception */
		throw ParserError(exception.what());
	}

	for(vector<Surface::Definition*>::iterator it = surfaceDefinition.begin() ; it != surfaceDefinition.end() ; ++it)
		delete (*it);
	/* Clean definitions, we don't need this anymore */
	for(vector<Cell::Definition*>::iterator it = cellDefinition.begin(); it != cellDefinition.end() ; ++it)
		delete (*it);
	for(vector<GeometricFeature::Definition*>::iterator it = featureDefinition.begin(); it != featureDefinition.end() ; ++it)
		delete (*it);

	/* Also clear the geometries containers */
	surfaceDefinition.clear();
	cellDefinition.clear();
	featureDefinition.clear();

	return geometry;
}

MaterialContainer* Parser::getMaterials() {
	/* Material container */
	MaterialContainer* materials = new MaterialContainer;

	try {
		/* Add surface into the geometry */
		materials->setupMaterials(materialDefinition);
	} catch (std::exception& exception) {
		/* Catch exception */
		throw ParserError(exception.what());
	}

	for(vector<Material::Definition*>::iterator it = materialDefinition.begin() ; it != materialDefinition.end() ; ++it)
		delete (*it);

	/* Clear the container */
	materialDefinition.clear();

	return materials;
}

Parser::~Parser() {

}

void tokenize(const string& str, vector<string>& tokens, const string& delimiters) {
	/* Skip delimiters at beginning */
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	/* Find first non-delimiter */
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		/* Found a token, add it to the vector */
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		/* Skip delimiters */
		lastPos = str.find_first_not_of(delimiters, pos);
		/* Find next non-delimiter */
		pos = str.find_first_of(delimiters, lastPos);
	}
}

const std::string trim(const std::string& pString,const std::string& pWhitespace) {
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == std::string::npos) {
        /* No content */
        return "";
    }
    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;
    return pString.substr(beginStr, range);
}

const std::string reduce(const std::string& pString,const std::string& pFill,const std::string& pWhitespace) {
    /* Trim first */
    std::string result(trim(pString, pWhitespace));

    /* Replace sub ranges */
    size_t beginSpace = result.find_first_of(pWhitespace);
    while (beginSpace != std::string::npos) {
        const size_t endSpace = result.find_first_not_of(pWhitespace, beginSpace);
        const size_t range = endSpace - beginSpace;
        result.replace(beginSpace, range, pFill);
        const size_t newStart = beginSpace + pFill.length();
        beginSpace = result.find_first_of(pWhitespace, newStart);
    }
    return result;
}

} /* namespace Helios */
