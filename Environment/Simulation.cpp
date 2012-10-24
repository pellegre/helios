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

#include "Simulation.hpp"

using namespace std;

namespace Helios {

void KeffSimulation::launch(const vector<CellParticle>& particles, vector<CellParticle>& fission_bank, Random& r) {

	/* --- Initialize geometry stuff */

	Surface* surface(0);  /* Surface pointer */
	bool sense(true);     /* Sense of the surface we are crossing */
	double distance(0.0); /* Distance to closest surface */

	for(vector<CellParticle>::const_iterator it = particles.begin() ; it != particles.end() ; ++it) {

		/* Get particle from the user supplied bank */
		pair<const Cell*,Particle> pc = (*it);
		const Cell* cell = pc.first;
		Particle particle = pc.second;

		/* Flag if particle is out of the system */
		bool out = false;

		while(true) {

			/* Get next surface and distance */
			cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

			/* Energy index of the particle */
			EnergyIndex energy_index = particle.eix();
			/* Get material */
			const Material* material = cell->getMaterial();
			/* Get total cross section */
			double mfp = material->getMeanFreePath(energy_index);

			/* Get collision distance */
			double collision_distance = -log(r.uniform())*mfp;

			while(collision_distance > distance) {
				/* Cut on a vacuum surface */
				if(surface->getFlags() & Surface::VACUUM) {
					out = true;
					break;
				}

				/* Transport the particle to the surface */
				particle.pos() = particle.pos() + distance * particle.dir();

				if(surface->getFlags() & Surface::REFLECTING) {
					/* Get normal */
					Direction normal;
					surface->normal(particle.pos(),normal);
					/* Reverse if necessary */
					if(sense == false) normal = -normal;
					/* Calculate the new direction */
					double projection = 2 * dot(particle.dir(), normal);
					particle.dir() = particle.dir() - projection * normal;
				} else {
					/* Now get next cell */
					surface->cross(particle.pos(),sense,cell);
					if(!cell) {
						cout << particle << endl;
						cout << *surface << endl;
					}
					/* Cut if the cell is dead */
					if(cell->getFlag() & Cell::DEADCELL) {
						out = true;
						break;
					}
				}

				/* Calculate new distance to the closest surface */
				cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

				/* Update material */
				material = cell->getMaterial();
				mfp = material->getMeanFreePath(energy_index);

				/* And the new collision distance */
				collision_distance = -log(r.uniform())*mfp;
			}

			if(out) break;

			/* If we are out, we reach some point inside a cell were a collision should occur */
			particle.pos() = particle.pos() + collision_distance * particle.dir();

			/* get and apply reaction to the particle */
			Reaction* reaction = material->getReaction(particle.eix(),r);
			(*reaction)(particle,r);

			if(particle.sta() == Particle::DEAD) break;
			if(particle.sta() == Particle::BANK) {
				population += particle.wgt();
				fission_bank.push_back(CellParticle(cell,particle));
				break;
			}
		}
	}
}


} /* namespace Helios */
