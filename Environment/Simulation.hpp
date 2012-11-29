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

#include "McEnvironment.hpp"

#include "../Tallies/Tally.hpp"

namespace Helios {

/* Class that launch a simulation from a bank of particles */
class Simulation {
protected:
	/* Environment (is where this class should look for the data) */
	const McEnvironment* environment;
	/* Local copy of the random number engine */
	Random base;
	/* Parameters for random number on simulations */
	size_t max_rng_per_history;
	/* Max samples when simulating the source */
	size_t max_samples;
	/* Particles per cycle */
	size_t particles_number;
	/* Reference to the source of the problem */
	Source* initial_source;
public:

	/* Initialize simulation */
	Simulation(const McEnvironment* environment);

	virtual ~Simulation() {/* */};
};

/*
 * KEFF simulation
 */
class KeffSimulation : public Simulation {

public:

	/* Type of cycle (active or inactive) */
	enum CycleType {
		INACTIVE = 0,
		ACTIVE   = 1
	};

	/* Initialize simulation */
	KeffSimulation(const McEnvironment* environment);

	/* Virtual function to simulate a batch of particles */
	virtual double simulateBank(TallyContainer& tallies) = 0;

	/* Virtual function to simulate the source of particles */
	virtual void simulateSource() = 0;

	/*
	 * Execute a a random walk of a particle in the current bank. If the simulation terminates with a
	 * a fission, banked particles are also set.
	 */
	double cycle(size_t nbank, const vector<ChildTally*>& child_tallies);

	/* Simulate a source particle and put it into the bank */
	void source(size_t nbank);

	/* Launch a simulation */
	void launch(CycleType type, TallyContainer& tallies);

	/* Get multiplication factor */
	double getKeff() const {return keff;}

	virtual ~KeffSimulation();

protected:
	/* KEFF estimation of one cycle */
	double keff;
	/* Global particle bank for this simulation */
	std::vector<CellParticle> fission_bank;
	/* Local bank on a cycle simulation */
	vector<vector<CellParticle> > local_bank;

	/* Current type of cycle */
	CycleType current_type;

	/* Estimators inside the cycle */
	enum Estimator {
		LEAK     = 0,
		ABS      = 1,
		N2N      = 2,
		N3N      = 3,
		N4N      = 4,
		KEFF_ABS = 5,
		KEFF_COL = 6,
		KEFF_TRK = 7
	};

	/* Accumulate estimator */
	template<Estimator Index>
	void estimate(const vector<ChildTally*>& tally_container, double value) {
		if(current_type == ACTIVE)
			tally_container[Index]->acc(value);
	}
};

template<class ParallelPolicy>
class ParallelKeffSimulation : public KeffSimulation, public ParallelPolicy {
public:
	ParallelKeffSimulation(const McEnvironment* environment) :
		KeffSimulation(environment) {
		/* Populate the particle bank with the initial source */
		simulateSource();
		/* Jump on base stream of RNGs */
		base.jump(particles_number * max_samples);
	};

	/* Method to simulate a batch of particles */
	double simulateBank(TallyContainer& tallies) {
		return ParallelPolicy::parallelBank(fission_bank.size(), this, tallies);
	}

	/* Method to simulate the source of particles */
	void simulateSource() {
		ParallelPolicy::parallelSource(fission_bank.size(), this);
	}

	virtual ~ParallelKeffSimulation() {/* */};
};

/* Single thread policy */
class SingleThread {
public:
	/* Parallel algorithm to fill the particle bank with the source */
	void parallelSource(size_t nbanks, KeffSimulation* simulation) {
		/* Populate the particle bank with the initial source */
		for(size_t i = 0 ; i < nbanks ; ++i)
			simulation->source(i);
	}
	/* Parallel algorihtm to simulate a bank of particles */
	double parallelBank(size_t nbanks, KeffSimulation* simulation, TallyContainer& tallies) {
		/* Total population */
		double total_population = 0.0;

		/* Initialize local tallies accumulators */
		vector<ChildTally*>& child_tallies = tallies.getChildTallies();

		/* Parallel loop to simulate the particle in the bank */
		for(size_t i = 0 ; i < nbanks ; ++i)
			total_population += simulation->cycle(i, child_tallies);

		/* Set tallies */
		tallies.setChildTallies(child_tallies);

		/* Return population */
		return total_population;
	}
};

/* OpenMP policy */
class OpenMp {
public:
	/* Parallel algorithm to fill the particle bank with the source */
	void parallelSource(size_t nbanks, KeffSimulation* simulation) {
		/* Populate the particle bank with the initial source */
		#pragma omp parallel for
		for(size_t i = 0 ; i < nbanks ; ++i)
			simulation->source(i);
	}
	/* Parallel algorihtm to simulate a bank of particles */
	double parallelBank(size_t nbanks, KeffSimulation* simulation, TallyContainer& tallies) {
		/* Total population */
		double total_population = 0.0;

		#pragma omp parallel
		{
			/* Local counter */
			double population = 0.0;

			/* Initialize local tallies accumulators */
			vector<ChildTally*>& child_tallies = tallies.getChildTallies();

			/* Parallel loop to simulate the particle in the bank */
			#pragma omp for
			for(size_t i = 0 ; i < nbanks ; ++i)
				population += simulation->cycle(i, child_tallies);

			/* Set tallies */
			tallies.setChildTallies(child_tallies);

			/* Update global population counter */
			#pragma omp critical
			{
				total_population += population;
			}

		}
		/* Return population */
		return total_population;
	}
};

/* IntelTbb policy */
class IntelTbb {
public:
	/* ---- Source simulator */

	class SourceSimulator {
		/* Simulation */
		KeffSimulation* simulation;
	public:
		SourceSimulator(KeffSimulation* simulation) : simulation(simulation) {/* */};
		void operator() (const tbb::blocked_range<size_t>& range) const {
			for(size_t i = range.begin() ; i < range.end() ; ++i)
				simulation->source(i);
		}
		~SourceSimulator() {/* */}
	};

	/* Parallel algorithm to fill the particle bank with the source */
	void parallelSource(size_t nbanks, KeffSimulation* simulation) {
		/* Populate the particle bank with the initial source */
		tbb::parallel_for(tbb::blocked_range<size_t>(0,nbanks),SourceSimulator(simulation));
	}

	/* ---- Power step simulator */

	class PowerStepSimulator {
		/* Simulation */
		KeffSimulation* simulation;
		TallyContainer& tallies;
	public:
		/* Population after the simulation */
		double local_population;

		PowerStepSimulator(KeffSimulation* simulation, TallyContainer& tallies) :
			simulation(simulation), tallies(tallies), local_population(0.0) {/* */};
		PowerStepSimulator(PowerStepSimulator& right, tbb::split) :
			simulation(right.simulation), tallies(right.tallies), local_population(0.0) {/* */}
		void join(PowerStepSimulator& right) {
			local_population += right.local_population;
		}
		void operator() (const tbb::blocked_range<size_t>& range) {
			/* Get population */
			double population = local_population;
			/* Initialize local tallies accumulators */
			vector<ChildTally*>& child_tallies = tallies.getChildTallies();
			/* Simulate cycle */
			for(size_t i = range.begin() ; i < range.end() ; ++i)
				population += simulation->cycle(i,child_tallies);
			/* Set tallies */
			tallies.setChildTallies(child_tallies);;
			/* Save new population */
			local_population = population;
		}
		~PowerStepSimulator() {/* */}
	};

	/* Parallel algorithm to simulate a bank of particles */
	double parallelBank(size_t nbanks, KeffSimulation* simulation, TallyContainer& tallies) {
		PowerStepSimulator power_step(simulation, tallies);
		/* Simulate power step */
		tbb::parallel_reduce(tbb::blocked_range<size_t>(0, nbanks), power_step);
		return power_step.local_population;
	}
};

} /* namespace Helios */
#endif /* SIMULATION_HPP_ */
