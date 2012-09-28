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

#include <iostream>
#include <string>
#include <vector>

#include "Parser/ParserTypes.hpp"
#include "Log/Log.hpp"
#include "Utils/Utils.hpp"

using namespace std;
using namespace Helios;

int main(int argc, char* argv[]) {

	/* Geometry */
	Geometry* geometry = new Geometry;
	/* Parser (XML for now) */
	Parser* parser = new XmlParser(*geometry);

	string filename = string(argv[1]);
	try {
		/* Read the input file */
		Log::ok() << "Reading file " + filename << Log::endl;
		parser->parseFile(filename);
	} catch(Parser::ParserError& parsererror) {
		Log::error() << "Error parsing file : " + filename + "." << Log::endl;
		/* Nothing to do, just print the message and exit */
		Log::error() << parsererror.what() << Log::endl;
		return 1;
	} catch(Parser::KeywordParserError& keyerror) {
		Log::error() << "Error parsing file : " + filename << Log::endl;
		/* Try to find the -bad- keyword */
		size_t line = seachKeyWords(filename,keyerror.getKeys());
		if(line)
			Log::error() << "Line " << (line + 1) << " : " << keyerror.what() << Log::endl;
		else
			Log::error() << keyerror.what() << Log::endl;
		return 1;
	}

	geometry->printGeo(std::cout);

	plot(*geometry,-3.6,3.6,-3.6,3.6,"test.png");

	/* Parameters */
	Direction dir(1,0,0);
	Coordinate pos(1.0,1.0,0);

	/* Geometry stuff */
	const Cell* cell(geometry->findCell(pos));
	Surface* surface(0);
	bool sense(true);
	double distance(0.0);
	cout << "[@] First cell : " << cell->getUserId() << endl;
	while(cell) {
		/* Get next surface and distance */
		cell->intersect(pos,dir,surface,sense,distance);
		if(!surface) break;
		/* Transport the particle */
		pos = pos + distance * dir;
		/* Now get next cell */
		surface->cross(pos,sense,cell);
		cout << "[@] Current cell : " << cell->getUserId() << endl;
		if(cell->getFlag() & Cell::DEADCELL) break;
	}

	delete geometry;
	delete parser;

	return 0;
}
