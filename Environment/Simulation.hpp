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
	/* Local copy of the random number engine */
	Random base;
	/* Environment (is where this class should look for the data) */
	McEnvironment* environment;

public:

	/* Parameters for random number on simulations */
	static size_t max_rng_per_history;

	/* Initialize simulation */
	Simulation(const Random& base, McEnvironment* environment);

	/* Launch a simulation */
	virtual void launch() = 0;

	virtual ~Simulation() {/* */};
};

/*
 * KEFF simulation
 */
class KeffSimulation : public Simulation {
	/* KEFF estimation of one cycle */
	double keff;
	/* Particles per cycle */
	size_t particles_number;
	/* Reference to the source of the problem */
	Source* initial_source;
	/* Global particle bank for this simulation */
	std::vector<CellParticle> fission_bank;
	/* Local bank on a cycle simulation */
	vector<vector<CellParticle> > local_bank;

	/* Accumulators */
	std::vector<Tally*> tallies;

public:
	/* Initialize simulation */
	KeffSimulation(const Random& random, McEnvironment* environment, double keff, size_t particles_number);

	/* Virtual function to simulate a batch of particles */
	virtual double simulateBank(size_t nbanks) = 0;

	/* Virtual function to simulate the source of particles */
	virtual void simulateSource(size_t nbanks) = 0;

	/* Get child tallies */
	void getTallies(vector<ChildTally*>& child_tallies) {
		child_tallies.resize(tallies.size());
		/* Create childs */
		for(size_t i = 0 ; i < child_tallies.size() ; ++i)
			child_tallies[i] = tallies[i]->getChild();
	}

	/* Set tallies */
	void setTallies(vector<ChildTally*>& child_tallies) {
		/* Sanity check */
		assert(child_tallies.size() == tallies.size());
		/* Join tallies */
		for(size_t i = 0 ; i < tallies.size() ; ++i)
			tallies[i]->join(child_tallies[i]);
	}

	/*
	 * Execute a a random walk of a particle in the current bank. If the simulation terminates with a
	 * a fission, banked particles are also set.
	 */
	double cycle(size_t nbank, const vector<ChildTally*>& child_tallies);

	/* Simulate a source particle and put it into the bank */
	void source(size_t nbank);

	/* Launch a simulation */
	void launch();

	/* Get multiplication factor */
	double getKeff() const {return keff;}

	virtual ~KeffSimulation();
};

template<class ParallelPolicy>
class ParallelKeffSimulation : public KeffSimulation, public ParallelPolicy {
public:
	ParallelKeffSimulation(const Random& random, McEnvironment* environment, double keff, size_t particles_number) :
		KeffSimulation(random, environment, keff, particles_number) {
		/* Populate the particle bank with the initial source */
		simulateSource(particles_number);
		/* Jump on base stream of RNGs */
		base.jump(particles_number * Source::max_samples);
	};

	/* Method to simulate a batch of particles */
	double simulateBank(size_t nbanks) {
		return ParallelPolicy::parallelBank(nbanks, this);
	}

	/* Method to simulate the source of particles */
	void simulateSource(size_t nbanks) {
		ParallelPolicy::parallelSource(nbanks, this);
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
	double parallelBank(size_t nbanks, KeffSimulation* simulation) {
		/* Total population */
		double total_population = 0.0;

		/* Initialize local tallies accumulators */
		vector<ChildTally*> tallies;
		simulation->getTallies(tallies);

		/* Parallel loop to simulate the particle in the bank */
		for(size_t i = 0 ; i < nbanks ; ++i)
			total_population += simulation->cycle(i, tallies);

		/* Set tallies */
		simulation->setTallies(tallies);

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
	double parallelBank(size_t nbanks, KeffSimulation* simulation) {
		/* Total population */
		double total_population = 0.0;

		#pragma omp parallel
		{
			/* Local counter */
			double population = 0.0;

			/* Initialize local tallies accumulators */
			vector<ChildTally*> tallies;
			simulation->getTallies(tallies);

			/* Parallel loop to simulate the particle in the bank */
			#pragma omp for
			for(size_t i = 0 ; i < nbanks ; ++i)
				population += simulation->cycle(i, tallies);

			/* Update global population counter */
			#pragma omp critical
			{
				total_population += population;
				/* Set tallies */
				simulation->setTallies(tallies);
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
		virtual ~SourceSimulator() {/* */}
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
	public:
		/* Population after the simulation */
		double local_population;
		/* Local tallies */
		vector<ChildTally*> tallies;

		PowerStepSimulator(KeffSimulation* simulation) :
			simulation(simulation), local_population(0.0) {
			/* Initialize local tallies accumulators */
			simulation->getTallies(tallies);
		};
		PowerStepSimulator(PowerStepSimulator& right, tbb::split) :
			simulation(right.simulation), local_population(0.0) {
			/* Initialize local tallies accumulators */
			simulation->getTallies(tallies);
		}
		void join(PowerStepSimulator& right) {
			local_population += right.local_population;
			/* Combine tallies */
			for(size_t i = 0 ; i < tallies.size() ; ++i)
				tallies[i]->join(right.tallies[i]);
		}
		void operator() (const tbb::blocked_range<size_t>& range) {
			/* Get population */
			double population = local_population;
			for(size_t i = range.begin() ; i < range.end() ; ++i)
				population += simulation->cycle(i,tallies);
			/* Save new population */
			local_population = population;
		}
		virtual ~PowerStepSimulator() {
			/* Erase tallies */
			for(size_t i = 0 ; i < tallies.size() ; ++i)
				delete tallies[i];
		}
	};

	/* Parallel algorithm to simulate a bank of particles */
	double parallelBank(size_t nbanks, KeffSimulation* simulation) {
		PowerStepSimulator power_step(simulation);
		/* Simulate power step */
		tbb::parallel_reduce(tbb::blocked_range<size_t>(0, nbanks), power_step);
		/* Set tallies */
		simulation->setTallies(power_step.tallies);
		return power_step.local_population;
	}
};

} /* namespace Helios */
#endif /* SIMULATION_HPP_ */
