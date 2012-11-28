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
#include <list>

#include "Parser/ParserTypes.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Cell.hpp"
#include "Material/Materials.hpp"
#include "Transport/Particle.hpp"
#include "Transport/Source.hpp"
#include "Common/Common.hpp"
#include "Environment/McEnvironment.hpp"
#include "Environment/Simulation.hpp"

using namespace std;
using namespace Helios;

int main(int argc, char **argv) {

	/* Check number of arguments */
	if(argc < 2) {
	  Helios::Log::error() << "Usage : " << argv[0] << " <filename>" << Helios::Log::endl;
	  exit(1);
	}

	/* Print header */
	Log::header();

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;

	/* Container of filenames */
	vector<string> input_files;

	while(*(++argv)) {
		string filename = string(*(argv));
		input_files.push_back(filename);
	}

	/* Environment */
	McEnvironment environment(parser);
	/* Parse files, to get the information to create the environment */
	environment.parseFiles(input_files);
	/* Setup the problem */
	environment.setup();

	/* Initialization - Random number */
	Random r(10);

	/* Initialization - KEFF cycle */
	double keff = 1.000;
	size_t neutrons = environment.getSetting<size_t>("criticality","particles");
	size_t skip = environment.getSetting<size_t>("criticality","inactive");
	size_t cycles = environment.getSetting<size_t>("criticality","batches") - skip; /* Active cycles */

	/* Initialize simulation */
	ParallelKeffSimulation<IntelTbb> simulation(r,&environment,keff,neutrons);

	for(size_t ncycle = 0 ; ncycle < skip ; ++ncycle) {
		simulation.launch(KeffSimulation::INACTIVE);
		/* Get multiplication factor */
		keff = simulation.getKeff();
		Log::color<Log::COLOR_BOLDRED>() << Log::ident(0) << " **** Cycle (Inactive) "
				<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << skip << Log::crst <<
				" keff = " << fixed << keff << Log::endl;
	}

	for(size_t ncycle = 0 ; ncycle < cycles ; ++ncycle) {
		Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << " **** Cycle (Active)   "
				<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << cycles << Log::endl;
		simulation.launch(KeffSimulation::ACTIVE);
	}

	delete parser;
	return 0;
}
