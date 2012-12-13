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

#ifndef SIMULATION_HPP_
#define SIMULATION_HPP_

#include <omp.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include "../McEnvironment.hpp"

#include "../../Tallies/Tally.hpp"

namespace Helios {

/* Class that launch a simulation from a bank of particles */
class SimulationBase {

protected:

	/* Environment (is where this class should look for the data) */
	const McEnvironment* environment;

	/* ---- RNG data */

	/* Local copy of the random number engine */
	Random base;
	/* Parameters for random number on simulations */
	size_t max_rng_per_history;
	/* Max samples when simulating the source */
	size_t max_samples;

	/* ---- Initial source */

	/* Reference to the source of the problem */
	Source* initial_source;

	/* ---- Simulation parameters (all simulations are divided in batches) */

	/* Number of batches */
	size_t nbatches;
	/* Number of particles in the batch */
	size_t nparticles;
	/* Number of inactive cycles (when the local tallies aren't accumulated) */
	size_t ninactive;

	/* Type of simulation (active or inactive) */
	enum SimulationType {
		INACTIVE = 0,
		ACTIVE   = 1
	};

	/* Current type of simulation */
	SimulationType simulation_type;

	/* ---- MPI stuff (a simulation could be distributed among nodes) */

	/* MPI communicator */
	const boost::mpi::communicator& local_comm;
	/* Local stride on the batch of particles */
	size_t local_stride;
	/* Number of particles inside this node */
	size_t local_particles;
	/* Local counters (this "tallies" should be synchronized after each active batch simulation) */
	TallyContainer active_tallies;
	/* Local counters (this "tallies" should be synchronized after each inactive batch simulation) */
	TallyContainer inactive_tallies;

	/* Simulate a batch of particles */
	void batch(SimulationType type);

	/* ---- Common methods for simulations */

	/* Accumulate estimator */
	template<size_t Index>
	void estimate(const vector<ChildTally*>& tally_container, double value);

	/* Reduce tallies */
	void reduceTallies(TallyContainer& local_tallies);

public:
	/* Initialize simulation */
	SimulationBase(const McEnvironment* environment, size_t nparticles, size_t nbatches, size_t ninactive = 0);

	/* ---- Global simulation methods */

	/* Simulate the entire source */
	virtual void simulateSource() = 0;

	/* Simulate a batch of particles */
	virtual void simulateBatch() = 0;

	/* ---- Local simulation methods */

	/* Simulate source if the n-th particle on the batch */
	virtual void source(size_t nbank) = 0;

	/* Simulate history of the n-th particle on the batch */
	virtual void history(size_t nbank, const std::vector<ChildTally*>& child_tallies) = 0;

	/* Update internal data before executing a batch of particles */
	virtual void beforeBatch() = 0;

	/* Update internal data after the batch simulation */
	virtual void afterBatch() = 0;

	/* Get child tallies */
	TallyContainer& getTallies();

	/* ---- Simulation methods */

	/* Launch a simulation */
	void launch();

	virtual ~SimulationBase() {/* */};
};

template<size_t Index>
void SimulationBase::estimate(const vector<ChildTally*>& tally_container, double value) {
	if(simulation_type == ACTIVE)
		tally_container[Index]->acc(value);
}

/* Generic class to launch a parallel simulation */
template<class SimulationClass, class ParallelPolicy>
class ParallelSimulation : public SimulationClass, public ParallelPolicy {
	using SimulationClass::nparticles;
	using SimulationClass::max_rng_per_history;
	using SimulationClass::local_particles;
	using SimulationClass::max_samples;
	using SimulationClass::base;
public:
	ParallelSimulation(const McEnvironment* environment) : SimulationClass(environment) {
		/* Populate the particle bank with the initial source */
		simulateSource();
	}

	/* Method to simulate a batch of particles */
	void simulateBatch() {
		ParallelPolicy::simulateBatch(local_particles, this);
		/* Jump on random number stream */
		base.jump(nparticles * max_rng_per_history);
	}

	/* Method to simulate the source of particles */
	void simulateSource() {
		ParallelPolicy::simulateSource(local_particles, this);
		/* Jump on base stream of RNGs */
		base.jump(nparticles * max_samples);
	}

};

/* Single thread policy */
class SingleThread {
public:
	/* Parallel algorithm to fill the particle bank with the source */
	void simulateSource(size_t nparticles, SimulationBase* simulation) {
		/* Populate the particle bank with the initial source */
		for(size_t i = 0 ; i < nparticles ; ++i)
			simulation->source(i);
	}
	/* Parallel algorihtm to simulate a bank of particles */
	void simulateBatch(size_t nparticles, SimulationBase* simulation) {

		/* Initialize local tallies accumulators */
		std::vector<ChildTally*>& child_tallies = simulation->getTallies().getChildTallies();

		/* Parallel loop to simulate the particle in the bank */
		for(size_t i = 0 ; i < nparticles ; ++i)
			simulation->history(i, child_tallies);

		/* Set tallies */
		simulation->getTallies().setChildTallies(child_tallies);
	}
};

/* OpenMP policy */
class OpenMp {
public:
	/* Parallel algorithm to fill the particle bank with the source */
	void simulateSource(size_t nparticles, SimulationBase* simulation) {
		/* Populate the particle bank with the initial source */
		#pragma omp parallel for
		for(size_t i = 0 ; i < nparticles ; ++i)
			simulation->source(i);
	}
	/* Parallel algorihtm to simulate a bank of particles */
	void simulateBatch(size_t nparticles, SimulationBase* simulation) {
		#pragma omp parallel
		{
			/* Initialize local tallies accumulators */
			std::vector<ChildTally*>& child_tallies = simulation->getTallies().getChildTallies();

			/* Parallel loop to simulate the particle in the bank */
			#pragma omp for
			for(size_t i = 0 ; i < nparticles ; ++i)
				simulation->history(i, child_tallies);

			/* Set tallies */
			simulation->getTallies().setChildTallies(child_tallies);
		}
	}
};

/* IntelTbb policy */
class IntelTbb {
public:
	/* ---- Source simulator */

	class SourceSimulator {
		/* Simulation */
		SimulationBase* simulation;
	public:
		SourceSimulator(SimulationBase* simulation) : simulation(simulation) {/* */};
		void operator() (const tbb::blocked_range<size_t>& range) const {
			for(size_t i = range.begin() ; i < range.end() ; ++i)
				simulation->source(i);
		}
		~SourceSimulator() {/* */}
	};

	/* Parallel algorithm to fill the particle bank with the source */
	void simulateSource(size_t nparticles, SimulationBase* simulation) {
		/* Populate the particle bank with the initial source */
		tbb::parallel_for(tbb::blocked_range<size_t>(0,nparticles), SourceSimulator(simulation));
	}

	/* ---- Power step simulator */

	class PowerStepSimulator {
		/* Simulation */
		SimulationBase* simulation;
		/* Tally container */
		TallyContainer& tallies;
	public:
		PowerStepSimulator(SimulationBase* simulation) :
			simulation(simulation), tallies(simulation->getTallies()){/* */};
		void operator() (const tbb::blocked_range<size_t>& range) const {
			/* Initialize local tallies accumulators */
			std::vector<ChildTally*>& child_tallies = tallies.getChildTallies();
			/* Simulate cycle */
			for(size_t i = range.begin() ; i < range.end() ; ++i)
				simulation->history(i,child_tallies);
			/* Set tallies */
			tallies.setChildTallies(child_tallies);;
		}
		~PowerStepSimulator() {/* */}
	};

	/* Parallel algorithm to simulate a bank of particles */
	void simulateBatch(size_t nparticles, SimulationBase* simulation) {
		/* Simulate power step */
		tbb::parallel_for(tbb::blocked_range<size_t>(0, nparticles), PowerStepSimulator(simulation));
	}
};

} /* namespace Helios */
#endif /* SIMULATION_HPP_ */
