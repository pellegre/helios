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

#ifndef PARTICLE_HPP_
#define PARTICLE_HPP_

#include "../Common/Common.hpp"

namespace Helios {

	class Particle {

	public:

		enum State {
			ALIVE = 0, /* The particle is alive and should be transport on next step */
			DEAD  = 1, /* The particle is dead, the transport is done for this one */
			BANK  = 2  /* The particle state should be banked on next step */
		};

		Particle() : position(Coordinate(0,0,0)), direction(Direction(0,0,0)), energy(EnergyPair(0,1.0)),
					 weight(1.0), state(ALIVE) {/* */}
		Particle(const Coordinate& position, const Direction& direction, const EnergyPair& energy,const double& weight) :
				 position(position), direction(direction), energy(energy), weight(weight), state(ALIVE) {/* */}

		~Particle() {/* */};

		/* Getters (constant) and setters */
		Direction getDirection() const {
			return direction;
		}

		void setDirection(Direction direction) {
			this->direction = direction;
		}

		EnergyPair getEnergy() const {
			return energy;
		}

		void setEnergy(EnergyPair energy) {
			this->energy = energy;
		}

		Coordinate getPosition() const {
			return position;
		}

		void setPosition(Coordinate position) {
			this->position = position;
		}

		double getWeight() const {
			return weight;
		}

		void setWeight(double weight) {
			this->weight = weight;
		}

		/* Get reference to internal data of the particle */
		Coordinate& pos() {return position;}
		Direction& dir() {return direction;}
		double& wgt() {return weight;}
		EnergyIndex& eix() {return energy.first;}
		Energy& evs() {return energy.second;}
		State& sta() {return state;}

	private:

		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Particle& q);

		/* Position on space */
		Coordinate position;

		/* Fly direction*/
		Direction direction;

		/* Energy pair (index and value). Sometimes both are used, sometimes only one */
		EnergyPair energy;

		/* Weight of the particle */
		double weight;

		/* State of the particle */
		State state;

	};

	/* Print a particle */
	std::ostream& operator<<(std::ostream& out, const Particle& q);

	/* Set an isotropic angle to the particle */
	void isotropicDirection(Direction& dir, Random& r);

} /* namespace Helios */
#endif /* PARTICLE_HPP_ */
