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

#include <signal.h>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "Parser/ParserTypes.hpp"
#include "Environment/McEnvironment.hpp"

using namespace std;
namespace mpi = boost::mpi;
namespace po = boost::program_options;

using namespace Helios;

/* Handling a CTRL-C */
void ctrl_c(int dummy) {
	/* Print message */
	Log::msg() << "Terminating Helios " << Log::endl;
	/* Close output file */
	Log::closeOutput();
	/* Exit from program */
	exit(1);
}

int main(int argc, char **argv) {

	/* Set the signal handler */
	signal(SIGINT,ctrl_c);

	/* Initialize MPI environment */
	mpi::environment env(argc, argv);
	mpi::communicator world;

	/* Set rank on the logger */
	Log::setRank(world.rank());

	/* Map of command line values */
    po::variables_map vm;

	/* Parse command line options */
	string optString;
	/* Generic options */
	po::options_description generic("Helios++ command line options");
	generic.add_options()
		;

	/* Declare a group of options that configure the plotter */
	po::options_description config("General options");
	config.add_options()
		("output,o", po::value<string>(&optString)->default_value("helios.output"),
				"output file")
		;

	/* Hidden options (input files) */
	po::options_description hidden("Hidden options");
	hidden.add_options()
		("input-file", po::value< vector<string> >(),
				"input file")
		;

	po::options_description cmdline_options;
	cmdline_options.add(generic).add(config).add(hidden);

	po::options_description visible("");
	visible.add(generic).add(config);

	po::positional_options_description p;
	p.add("input-file", -1);

	try {

        store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);

        if (vm.size() == 0) {
        	Log::header();
        	Log::msg() << visible << endl;
            return 0;
        }

    }
    catch(exception& e)
    {
		Log::error() << "Command line error : " << e.what() << Log::endl;
		return 1;
    }

	/* Container of filenames */
    vector<string> input_files;
    if(vm.count("input-file"))
    	input_files = vm["input-file"].as< vector<string> >();
    else {
    	Log::header();
    	Log::msg() << visible << endl;
    	Log::msg() << "Example : " << string(argv[0]) << " --output file.dat input1.xml input2.xml ... inputN.xml" << Log::endl;
    	return 1;
    }

	/* Get output file name */
	string output = vm["output"].as<string>();

	/* Print general information on master node */
	if(world.rank() == 0) {
		/* Print header */
		Log::header(Log::msg());

		/* Open output file from the logger */
		Log::setOutput(output);

		/* Print header inside the output file */
		Log::header(Log::fout(), false);
	}

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;

	/* Start timing to get total elapsed time */
	mpi::timer total_time;

	/* Environment */
	McEnvironment environment(parser);
	try {

		/* Parse files, to get the information to create the environment */
		environment.parseFiles(input_files);

		/* Setup the problem */
		environment.setup();

		/* Once the problem is setup, print some stuff on the output file */
		if(world.rank() == 0) {
			Log::printLine(Log::fout(), "*");
			Log::fout() << endl << endl << "[#] Materials module" << endl << endl;
			environment.getModule<Materials>()->print(Log::fout());

			/* ACE module may not be loaded on Macro-XS calculations */
			try {
				Log::printLine(Log::fout(), "*");
				Log::fout() << endl << endl << "[#] Ace module" << endl << endl;
				environment.getModule<AceModule>()->print(Log::fout());
			} catch(exception& e) {/* */}

			Log::printLine(Log::fout(), "*");
			Log::fout() << endl << endl << "[#] Source module" << endl << endl;
			environment.getModule<Source>()->print(Log::fout());

			Log::printLine(Log::fout(), "*");
			Log::fout() << endl << endl << "[#] General settings" << endl << endl;
			environment.getModule<Settings>()->print(Log::fout());
		}

		/* Launch simulation */
		environment.simulate(world);

	} catch(exception& error) {
		/* Print error message and return */
		Log::error() << error.what() << Log::endl;
		return 1;
	}

	/* Print total time on master node */
	if (world.rank() == 0) {
		/* Final time on console */
		Log::msg()  << left << "Total time elapsed     : " << total_time.elapsed() << " seconds " << Log::endl;

		/* Final time on output file */
		Log::fout() << endl;
		Log::fout() << left << "Total time elapsed     : " << total_time.elapsed() << " seconds " << endl;
	}

	delete parser;
	return 0;
}
