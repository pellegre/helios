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

#include <boost/mpi.hpp>
#include <numeric>

#include "McEnvironment.hpp"
#include "Simulation.hpp"
#include "../Tallies/Tally.hpp"

using namespace std;
namespace mpi = boost::mpi;

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

	/* Get number of MPI nodes */
	size_t nodes = world.size();

	/* Divide number of particles */
	size_t local_particles = neutrons / nodes;

	/* Check non-integer division */
	int extra_particles = neutrons % nodes;
	if(world.rank() < extra_particles)
		local_particles++;

	/* Get extra particles from other node (to calculate the stride) */
	std::vector<size_t> all_bank_sizes;
	mpi::all_gather(world, local_particles, all_bank_sizes);

	/* Calculate local stride on the fission bank (calculating extra particles) */
	size_t local_stride(0);
	if(world.rank() != 0)
		local_stride = accumulate(all_bank_sizes.begin(), all_bank_sizes.begin() + world.rank(), 0);

	/* Print data of the simulation */
	Log::bok() << "Launching simulation " << Log::endl;
	Log::msg() << left << Log::ident(1) << " - RNG seed                : " << seed << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of particles     : " << neutrons << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of active cycles : " << cycles << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Number of MPI nodes     : " << nodes << Log::endl;
	Log::msg() << left << Log::ident(1) << " - Multithreading          : " << multithread << Log::endl;
	Log::msg() << Log::endl;

	/* Print simulation data on the output file */
	Log::printLine(Log::fout(), "*");
	Log::fout() << endl << endl << "[#] Simulation" << endl << endl;
	Log::fout() << " - RNG seed                : " << seed << endl;
	Log::fout() << " - Number of particles     : " << neutrons << endl;
	Log::fout() << " - Number of active cycles : " << cycles << endl;
	Log::fout() << " - Number of MPI nodes     : " << nodes << endl;
	Log::fout() << " - Multithreading          : " << multithread << endl;

	/* Create simulation */
	if(multithread == "tbb")
		simulation = new ParallelKeffSimulation<IntelTbb>(this, local_particles, local_stride);
	else if(multithread == "omp")
		simulation = new ParallelKeffSimulation<OpenMp>(this, local_particles, local_stride);
	else if(multithread == "single")
		simulation = new ParallelKeffSimulation<SingleThread>(this, local_particles, local_stride);
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

	for(size_t ncycle = 0 ; ncycle < skip ; ++ncycle) {

		/* Simulate and get the total population */
		double local_population = simulation->launch(KeffSimulation::INACTIVE, tallies);

		/* ---- Get data from nodes */

		/* Reduce total population */
		double total_population(0.0);
		mpi::all_reduce(world, local_population, total_population, std::plus<double>());

		/* Update information (preparing for next cycle). The fission bank gets updated here with the new particles */
		simulation->update(total_population, neutrons);

		/* Get the bank size (after the simulaton of the current cycle) */
		size_t local_bank_size = simulation->bankSize();

		/* Gather fission bank size to create new strides (this is the number of particles at the end of this cycle) */
		mpi::all_gather(world, local_bank_size, all_bank_sizes);

		/* Update number of particles */
		neutrons = accumulate(all_bank_sizes.begin(), all_bank_sizes.end(), 0);

		/* Update stride of the current local simulation */
		if(world.rank() == 0)
			simulation->setStride(0);
		else
			simulation->setStride(accumulate(all_bank_sizes.begin(), all_bank_sizes.begin() + world.rank(), 0));

		/* Get multiplication factor */
		double keff = simulation->getKeff();

		/* Print information */
		Log::color<Log::COLOR_BOLDRED>() << Log::ident(0) << " **** Cycle (Inactive) "
				<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << skip << Log::crst
				<< " keff = " << fixed << keff << Log::endl;

	}

	double average_time(0.0);
	for(size_t ncycle = 0 ; ncycle < cycles ; ++ncycle) {
		/* Initialize timer for the cycle */
		mpi::timer cycle_time;

		/* Print information (on master node) */
		if (world.rank() == 0) {
			Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << " **** Cycle (Active)   "
					<< setw(4) << right << ncycle + 1 << " / " << setw(4) << left << cycles << Log::endl;
		}

		/* Launch active simulation */
		double local_population = simulation->launch(KeffSimulation::ACTIVE, tallies);

		/* ---- Reduce tallies */

		/* Reduce the tallies */
		tallies.reduce();

		if (world.rank() == 0) {
			/* TODO - (this is *too* naive) Get all tallies from slaves */
			for(int i = 1 ; i < world.size() ; ++i) {
				TallyContainer slave_tallies;
				world.recv(i, 0, slave_tallies);
				tallies.join(slave_tallies);
			}
			/* Accumulate tallies on the master */
			tallies.accumulate(neutrons);

			/* Print tallies (only on master) */
			for(TallyContainer::const_iterator it = tallies.begin() ; it != tallies.end() ; ++it) {
				(*it)->print(Log::msg());
				Log::msg() << Log::endl;
			}
		} else {
			/* Send tally container to the master node */
			world.send(0, 0, tallies);
			/* And clear it */
			tallies.clear();
		}

		/* ---- Get data from nodes */

		/* Reduce total population */
		double total_population(0.0);
		mpi::all_reduce(world, local_population, total_population, std::plus<double>());

		/* Update information (preparing for next cycle). The fission bank gets updated here with the new particles */
		simulation->update(total_population, neutrons);

		/* Get the bank size (after the simulaton of the current cycle) */
		size_t local_bank_size = simulation->bankSize();

		/* Gather fission bank size to create new strides (this is the number of particles at the end of this cycle) */
		mpi::all_gather(world, local_bank_size, all_bank_sizes);

		/* ---- Update internal data of the simulation */

		/* Update number of particles */
		neutrons = accumulate(all_bank_sizes.begin(), all_bank_sizes.end(), 0);

		/* Update stride of the current local simulation */
		if(world.rank() == 0)
			simulation->setStride(0);
		else
			simulation->setStride(accumulate(all_bank_sizes.begin(), all_bank_sizes.begin() + world.rank(), 0));

		if (world.rank() == 0) {
			double time_elapsed = cycle_time.elapsed();
			/* Print time on master node */
			Log::msg() << Log::endl;
			Log::msg() << "Time elapsed in this cycle : " << time_elapsed << " seconds " << Log::endl;
			Log::msg() << Log::endl;
			/* Accumulate average time */
			average_time += time_elapsed;
		}
	}

	if (world.rank() == 0) {
		/* Print data on console */
		Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << "End simulation on " << Log::date() << Log::endl;
		Log::msg() << left << "Average time per cycle : " << average_time / cycles << " seconds " << Log::endl;

		/* Put final estimation on output file */
		Log::fout() << endl << "End simulation on " << Log::date() << endl;
		Log::fout() << "Average time per cycle : " << average_time / cycles << " seconds " << endl;
		Log::fout() << endl << "Final estimation " << endl << endl;
		/* Print tallies (only on master) */
		for(TallyContainer::const_iterator it = tallies.begin() ; it != tallies.end() ; ++it) {
			(*it)->print(Log::fout());
			Log::fout() << endl;
		}
	}

	delete simulation;
}

} /* namespace Helios */
