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

#ifndef ANALOGKEFF_HPP_
#define ANALOGKEFF_HPP_

#include "Simulation.hpp"

namespace Helios {

class AnalogKeff: public Helios::SimulationBase {
	/* KEFF estimation of one cycle */
	double keff;
	/* Initial number of particles */
	size_t particles_number;
	/* Global particle bank for this simulation */
	std::vector<CellParticle> fission_bank;
	/* Local bank on a cycle simulation */
	vector<vector<CellParticle> > local_bank;

	/* Transport a particle through void cells until a material is found or the particle get out of the system */
	bool voidTransport(const Material*& material, Particle& particle, const Cell*& cell);

	/* Estimators inside the cycle */
	enum Estimator {
		POP      = 0,
		LEAK     = 1,
		ABS      = 2,
		KEFF_ABS = 3,
		KEFF_COL = 4,
		KEFF_TRK = 5,
	};

public:
	AnalogKeff(const McEnvironment* environment);

	/* ---- Local simulation methods */

	/* Simulate source if the n-th particle on the batch */
	void source(size_t nbank);

	/* Simulate history of the n-th particle on the batch */
	void history(size_t nbank, const std::vector<ChildTally*>& child_tallies);

	/* Update internal data before executing a batch of particles */
	void beforeBatch();

	/* Update internal data after the batch simulation */
	void afterBatch();

	virtual ~AnalogKeff();
};

} /* namespace Helios */
#endif /* ANALOGKEFF_HPP_ */
