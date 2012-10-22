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

#include "McEnvironment.hpp"

using namespace std;

namespace Helios {

McEnvironment::McEnvironment(Parser* parser) : parser(parser) {
	/* Register the module factories */
	registerFactory(new MaterialsFactory(this));
	registerFactory(new GeometryFactory(this));
	registerFactory(new SourceFactory(this));
}

void McEnvironment::parseFile(const std::string& filename) {
	vector<string> input;
	input.push_back(filename);
	parseFiles(input);
}

void McEnvironment::parseFiles(const std::vector<std::string>& input_files) {
	/* Check if there is a parser on the environment */
	if (!parser)
		throw(GeneralError("Attempt to parse a file without a parser loaded on the environment"));

	/* Parse information */
	for(vector<string>::const_iterator it = input_files.begin() ; it != input_files.end() ; ++it)
		parser->parseFile((*it));
	/* Get parsed objects */
	vector<McObject*> objects = parser->getObjects();
	/* Put the on the map */
	for(vector<McObject*>::iterator it = objects.begin() ; it != objects.end() ; ++it)
		object_map[(*it)->getModuleName()].push_back((*it));
}

McEnvironment::~McEnvironment() {
	/* Clear factories and modules */
	for(map<string,ModuleFactory*>::iterator it = factory_map.begin() ; it != factory_map.end() ; ++it)
		delete (*it).second;
	for(map<std::string,McModule*>::iterator it = module_map.begin() ; it != module_map.end() ; ++it)
		delete (*it).second;
}

void McEnvironment::setup() {
	/* First, we need to setup the materials (i.e. materials) */
	setupModule<Materials>();

	/* Once materials are setup, we need to setup the geometry (so cells can grab materials from the environment)*/
	setupModule<Geometry>();

	/* Finally, we setup the source */
	setupModule<Source>();

	/* Clean all definitions */
	for(map<string,vector<McObject*> >::iterator it = object_map.begin() ; it != object_map.end() ; ++it) {
		purgePointers((*it).second);
	}
	/* Clear map */
	object_map.clear();
}

} /* namespace Helios */
