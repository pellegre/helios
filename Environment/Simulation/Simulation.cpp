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
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Simulation.hpp"

using namespace std;
namespace mpi = boost::mpi;

namespace Helios {

SimulationBase::SimulationBase(const McEnvironment* environment, size_t nparticles, size_t nbatches, size_t ninactive) :
		environment(environment),
		base(environment->getSetting<long unsigned int>("seed","value")),
		max_rng_per_history(environment->getSetting<size_t>("max_rng_per_history","value")),
		max_samples(environment->getSetting<size_t>("max_source_samples","value")),
		initial_source(environment->getModule<Source>()),
		nbatches(nbatches), nparticles(nparticles), ninactive(ninactive), simulation_type(INACTIVE),
		local_comm(environment->getCommunicator()),
		local_stride(0) {
	/* Calculate local number of particles and set the stride on the random number generator */
	size_t nodes = local_comm.size();

	/* Divide number of particles */
	local_particles = nparticles / nodes;

	/* Check non-integer division */
	int extra_particles = nparticles % nodes;
	if(local_comm.rank() < extra_particles)
		local_particles++;

	/* Get extra particles from other node (to calculate the stride) */
	std::vector<size_t> all_bank_sizes;
	mpi::all_gather(local_comm, local_particles, all_bank_sizes);

	/* Calculate local stride on this node (calculating extra particles) */
	if(local_comm.rank() != 0)
		local_stride = accumulate(all_bank_sizes.begin(), all_bank_sizes.begin() + local_comm.rank(), 0);
}

void SimulationBase::reduceTallies(TallyContainer& local_tallies) {
	/* Reduce the tallies */
	local_tallies.reduce();

	if (local_comm.rank() == 0) {
		/* TODO - (this is *too* naive) Get all tallies from slaves */
		for(int proc = 1 ; proc < local_comm.size() ; ++proc) {
			TallyContainer slave_tallies;
			local_comm.recv(proc, 0, slave_tallies);
			local_tallies.join(slave_tallies);
		}
		/* Accumulate tallies on the master */
		local_tallies.accumulate(nparticles);
		/* Print tallies (only on master) */
		for(TallyContainer::const_iterator it = local_tallies.begin() ; it != local_tallies.end() ; ++it) {
			(*it)->print(Log::msg());
			Log::msg() << Log::endl;
		}
	} else {
		/* Send tally container to the master node */
		local_comm.send(0, 0, local_tallies);
		/* And clear it */
		local_tallies.clear();
	}
}

TallyContainer& SimulationBase::getTallies() {
	if(simulation_type == INACTIVE)
		return inactive_tallies;
	return active_tallies;
}

void SimulationBase::batch(SimulationType type) {
	/* Set current type */
	simulation_type = type;

	/* Make internal updates before simulating the batch */
	beforeBatch();

	/* Simulate the batch of particles */
	simulateBatch();

	/* ---- Reduce tallies */
	if(simulation_type == INACTIVE) reduceTallies(inactive_tallies);
	else if(simulation_type == ACTIVE) reduceTallies(active_tallies);

	/* Update internal data */
	afterBatch();
}

void SimulationBase::launch() {
	/* Simulate inactive batches */
	for(size_t i = 0 ; i < ninactive ; ++i) {
		/* Print information */
		Log::color<Log::COLOR_BOLDRED>() << Log::ident(0) << " **** Batch (Inactive) "
				<< setw(4) << right << i + 1 << " / " << setw(4) << left << ninactive << Log::endl;

		/* Simulate batch */
		batch(INACTIVE);
	}

	/* Get number of active nactive */
	size_t nactive = nbatches - ninactive;

	/* Average time per cycle */
	double average_time(0.0);
	/* Average particle rate per cycle */
	double average_rate(0.0);
	/* Simulate active batches */
	for(size_t i = 0 ; i < nactive ; ++i) {
		/* Initialize timer for the cycle */
		mpi::timer cycle_time;

		/* Print information */
		Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << " **** Batch (Active)   "
				<< setw(4) << right << i + 1 << " / " << setw(4) << left << nactive << Log::endl;

		/* Simulate batch */
		batch(ACTIVE);

		/* Get time elapsed */
		double time_elapsed = cycle_time.elapsed();

		/* Print time on master node */
		Log::msg() << Log::endl;
		Log::msg() << "Time elapsed in this batch : " << time_elapsed << " seconds " << Log::endl;
		Log::msg() << Log::endl;

		/* Accumulate average time */
		average_time += time_elapsed;
		/* Accumulate average rate */
		average_rate += nparticles / time_elapsed;
	}

	/* Print data on console */
	Log::color<Log::COLOR_BOLDWHITE>() << Log::ident(0) << "End simulation on " << Log::date() << Log::endl;
	Log::msg() << left << "Average time per cycle : " << average_time / nactive << " seconds " << Log::endl;
	Log::msg() << left << "Average neutrons / sec : " << average_rate / (1000 * nactive) << " K neutrons / sec " << endl;

	/* Put final estimation on output file */
	Log::fout() << endl << "End simulation on " << Log::date() << endl;
	Log::fout() << "Average time per cycle : " << average_time / nactive << " seconds " << endl;
	Log::fout() << "Average neutrons / sec : " << average_rate / (1000 * nactive) << " K neutrons / sec " << endl;
	Log::fout() << endl << "Final estimation " << endl << endl;
	/* Print tallies (only on master) */
	for(TallyContainer::const_iterator it = active_tallies.begin() ; it != active_tallies.end() ; ++it) {
		(*it)->print(Log::fout());
		Log::fout() << endl;
	}
}

} /* namespace Helios */
