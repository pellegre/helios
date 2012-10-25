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

#include "McEnvironment.hpp"

namespace Helios {

/* Class that launch a simulation from a bank of particles */
class Simulation {
protected:
	/* Local copy of the random number engine */
	Random base;
	/* Environment (is where this class should look for the data) */
	McEnvironment* environment;

	/* Parameters for random number on simulations */
	size_t max_rng_per_source;
	size_t max_rng_per_history;
public:

	/* Pair of particle and cell */
	typedef std::pair<InternalCellId,Particle> CellParticle;

	/* Initialize simulation */
	Simulation(const Random& base,McEnvironment* environment);

	/* Launch a simulation */
	virtual void launch() = 0;

	virtual ~Simulation() {/* */};
};

/*
 * KEFF simulation
 *
 * Apart of accumulate the user defined tallies, this class also creates a bank
 * of particles representing the source fission (as a result of this simulation).
 * Is caller responsibility do whatever he/she wants with it.
 */
class KeffSimulation : public Simulation {
	/* Population after the simulation */
	double keff;
	/* Particles per cycle */
	size_t particles_number;
	/* Global particle bank for this simulation */
	std::vector<CellParticle> fission_bank;
	/* Reference to the geometry of the problem */
	Geometry* geometry;
public:
	/* Initialize simulation */
	KeffSimulation(const Random& random, McEnvironment* environment, double keff, size_t particles_number);

	/* Launch a simulation */
	void launch();

	/* Get multiplication factor */
	double getKeff() const {return keff;}

	virtual ~KeffSimulation() {/* */};
};

} /* namespace Helios */
#endif /* SIMULATION_HPP_ */
