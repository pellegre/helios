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

#ifndef ISOTOPE_HPP_
#define ISOTOPE_HPP_

#include "../Common/Common.hpp"
#include "../Transport/Particle.hpp"

namespace Helios {

	/* A reaction is simply a functor that change the state of a particle (based on a random number generator) */
	class Reaction {
	public:
		Reaction() {/* */}
		virtual void operator() (Particle& particle, Random& r) const = 0;
		virtual ~Reaction() {/* */}
	};

	class Isotope {
	protected:
		/*
		 * Flag if the isotope is fissile, should be set on child's constructor
		 * By default is false.
		 */
		bool fissile;
	public:
		Isotope() : fissile(false) {/* */};

		/*
		 * -- Get absorption probability
		 *
		 * Not disappearance, this include fission if is present on the isotope
		 */
		virtual double getAbsorptionProb(Energy& energy) const = 0;

		/* -- Check if the isotope is fissile */
		bool isFissile() const {return fissile;}

		/* -- Get fission probability */
		virtual double getFissionProb(Energy& energy) const = 0;

		/*
		 * -- Fission.
		 *
		 * The fission always should be treated separately... The user
		 * could decide to not produce neutrons from fission on a simulation.
		 * We also need to bank particles with a fission distribution on
		 * KEFF simulations.
		 */
		virtual void fission(Particle& particle, Random& random) const = 0;

		/*
		 * -- Scatter
		 *
		 * This method should change the particle's phase space coordinates
		 * according to a secondary angle distributions and/or a secondary energy
		 * distribution.
		 */
		virtual void scatter(Particle& particle, Random& random) const = 0;

		virtual ~Isotope() {/* */};
	};

} /* namespace Helios */
#endif /* ISOTOPE_HPP_ */
