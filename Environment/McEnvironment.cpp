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
#include "Simulation.hpp"
#include "../Tallies/Tally.hpp"

using namespace std;

namespace Helios {

McEnvironment::McEnvironment(Parser* parser) : parser(parser) {
	/* Register the module factories */
	registerFactory(new SettingsFactory(this));
	registerFactory(new MaterialsFactory(this));
	registerFactory(new AceFactory(this));
	registerFactory(new GeometryFactory(this));
	registerFactory(new SourceFactory(this));

	/* Add some common default values for some settings */
	pushObject(new SettingsObject("max_source_samples", "100"));
	pushObject(new SettingsObject("max_rng_per_history", "100000"));
	pushObject(new SettingsObject("multithread", "tbb"));
	pushObject(new SettingsObject("seed", "10"));
	pushObject(new SettingsObject("energy_freegas_threshold", "400.0"));
	pushObject(new SettingsObject("awr_freegas_threshold", "1.0"));
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
	/* Put the objects on the map */
	for(vector<McObject*>::iterator it = objects.begin() ; it != objects.end() ; ++it) {
		/* Set the environment where this object is passing through */
		(*it)->setEnvironment(this);
		object_map[(*it)->getModuleName()].push_back((*it));
	}
}

McEnvironment::~McEnvironment() {
	/* Clear factories and modules */
	for(map<string,ModuleFactory*>::iterator it = factory_map.begin() ; it != factory_map.end() ; ++it)
		delete (*it).second;
	for(map<std::string,McModule*>::iterator it = module_map.begin() ; it != module_map.end() ; ++it)
		delete (*it).second;
	/* Clean all definitions */
	for(map<string,vector<McObject*> >::iterator it = object_map.begin() ; it != object_map.end() ; ++it)
		purgePointers((*it).second);
	/* Clear map */
	object_map.clear();
}

void McEnvironment::setup() {
	/* Setup Settings module */
	setupModule<Settings>();

	/* Setup the Ace module */
	setupModule<AceModule>();

	/* Setup the materials module */
	setupModule<Materials>();

	/* Once materials are setup, we need to setup the geometry module (so cells can grab materials from the environment) */
	setupModule<Geometry>();

	/* Finally, we setup the source module */
	setupModule<Source>();
}

void McEnvironment::simulate(boost::mpi::communicator& world) const {
	/* Simulation pointer */
	KeffSimulation* simulation(0);

	/* Get multithread type of simulation */
	string multithread = getSetting<string>("multithread", "value");
	/* Number of particles */
	size_t neutrons = getSetting<size_t>("criticality","particles");
	/* Get number of inactive cycles */
	size_t skip = getSetting<size_t>("criticality","inactive");
	/* Active cycles */
	size_t cycles = getSetting<size_t>("criticality","batches") - skip;
	/* Random number seed */
	long unsigned int seed = getSetting<long unsigned int>("seed","value");

	/* Create simulation */
	Log::bok() << "Launching simulation " << Log::endl;
	Log::msg() << left << Log::ident(1) << " - RNG seed                : " << seed << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of particles     : " << neutrons << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of active cycles : " << cycles << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of MPI nodes     : " << world.size() << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Multithreading          : " << multithread << Log::endl;

	if(multithread == "tbb")
		simulation = new ParallelKeffSimulation<IntelTbb>(this, 0);
	else if(multithread == "omp")
		simulation = new ParallelKeffSimulation<OpenMp>(this, 0);
	else if(multithread == "single")
		simulation = new ParallelKeffSimulation<SingleThread>(this, 0);
	else
		throw(GeneralError("Multithreading type " + multithread + " not recognized"));


	/* Create some tallies for this simulation */
	TallyContainer tallies;
	/* Leakage */
	tallies.pushTally(new Tally("leakage"));
	/* Absorptions */
	tallies.pushTally(new Tally("absorption"));

	/* (n,2n) */
	tallies.pushTally(new Tally("(n,2n)"));
	/* (n,3n) */
	tallies.pushTally(new Tally("(n,3n)"));
	/* (n,4n) */
	tallies.pushTally(new Tally("(n,4n)"));

	/* Absorption KEFF */
	tallies.pushTally(new Tally("keff (abs)"));
	/* Collision KEFF */
	tallies.pushTally(new Tally("keff (col)"));
	/* Track length KEFF */
	tallies.pushTally(new Tally("keff (trk)"));

	double ave = 0.0;
	for(size_t ncycle = 0 ; ncycle < skip ; ++ncycle) {

		/* Simulate and get the total population */
		double total_population = simulation->launch(KeffSimulation::INACTIVE, tallies);

		/* Get the bank size (after the simulaton of the current cycle) */
		size_t bank_size = simulation->bankSize();

		/* Update information (preparing for next cycle) */
		simulation->update(total_population, bank_size, 0);

		/* Get multiplication factor */
		double keff = simulation->getKeff();

		/* Print information */
		Log::color<Log::COLOR_BOLDRED>() << Log::ident(0) << " **** Cycle (Inactive) "
				<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << skip << Log::crst << " keff = " << fixed << keff << Log::endl;

		ave += keff;
	}

	cout << "Average = " << ave / skip << endl;

	for(size_t ncycle = 0 ; ncycle < cycles ; ++ncycle) {
		/* Print information */
		Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << " **** Cycle (Active)   "
				<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << cycles << Log::endl;

		simulation->launch(KeffSimulation::ACTIVE, tallies);
	}

	delete simulation;
}

} /* namespace Helios */
