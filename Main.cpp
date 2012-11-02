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

	environment.getModule<Materials>()->printMaterials(cout);

	/* Initialization - Random number */
	Random r;
	r.seed(1);

	/* Initialization - KEFF cycle */
	double keff = 1.186;
	double ave_keff = 0.0;
	int neutrons = 10000;
	int skip = 50;
	int cycles = 250;

	/* Initialize simulation */
	OpenMp::KeffSimulation simulation(r,&environment,keff,neutrons);

	for(int ncycle = 0 ; ncycle <= cycles ; ++ncycle) {

		simulation.launch();

		/* Calculate multiplication factor */
		keff = simulation.getKeff();

		if(ncycle > skip) {
			Log::ok() << "Cycle = " << ncycle << " - keff = " << keff << Log::endl;
			ave_keff += keff;
		} else {
			Log::ok() << " Cycle (Inactive) = " << ncycle << " - keff = " << keff << Log::endl;
		}

	}

	Log::ok() << " Final keff = "<< ave_keff/ (double)(cycles - skip) << Log::endl;

	delete parser;
	return 0;
}
