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
		friend std::ostream& operator<<(std::ostream& out, const Reaction& q);
		/* Internal ID of a reaction */
		InternalId internal_id;
	public:
		Reaction(InternalId internal_id) : internal_id(internal_id) {/* */}
		/* Get internal ID */
		InternalId getId() const {return internal_id;}
		/* Sample reaction */
		virtual void operator() (Particle& particle, Random& r) const = 0;
		/* Print information about the reaction (by default it does nothing) */
		virtual void print(std::ostream& out) const = 0;
		virtual ~Reaction() {/* */}
	};

	/* Print a reaction */
	std::ostream& operator<<(std::ostream& out, const Reaction& q);

	class Isotope {
	protected:
		/*
		 * Flag if the isotope has fission information, should be set on child's constructor
		 * By default is false.
		 */
		bool fissile;

		/* Print isotope */
		virtual void print(std::ostream& out) const {/* Nothing by default */}
		friend std::ostream& operator<<(std::ostream& out, const Isotope& q);

		/* Internal identification of this isotope */
		InternalIsotopeId internal_id;

		/* User identification of this isotope */
		IsotopeId user_id;

	public:

		Isotope(const IsotopeId& user_id) : fissile(false), user_id(user_id) {/* */};

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
		/* -- Get NU-bar from fission reaction */
		virtual double getNuBar(const Energy& energy) const = 0;

		/* -- Get elastic probability */
		virtual double getElasticProb(Energy& energy) const = 0;

		/*
		 * -- Fission.
		 *
		 * The fission always should be treated separately... The user
		 * could decide to not produce neutrons from fission on a simulation.
		 * We also need to bank particles with a fission distribution on
		 * KEFF simulations.
		 */
		virtual Reaction* fission() const = 0;

		/*
		 * -- Elastic scattering
		 *
		 * Get elastic scattering reaction of this isotope.
		 */
		virtual Reaction* elastic() const = 0;

		/*
		 * -- Inelastic Scattering
		 *
		 * This method should return a reaction that change the particle's phase space coordinates
		 * according to a secondary angle distributions and/or a secondary energy distribution. This
		 * method should not include elastic scattering between those reactions. The sampling is
		 * done using the particle energy.
		 */
		virtual Reaction* inelastic(Energy& energy, Random& random) const = 0;

		/* Set internal / unique identifier for the isotope */
		void setInternalId(const InternalMaterialId& internal) {internal_id = internal;}
		/* Return the internal ID associated with this isotope. */
		const InternalMaterialId& getInternalId() const {return internal_id;}

		/* Get user ID of the isotope */
		IsotopeId getUserId() const {return user_id;}

		virtual ~Isotope() {/* */};
	};

	/* Output isotope information */
	std::ostream& operator<<(std::ostream& out, const Isotope& q);

} /* namespace Helios */
#endif /* ISOTOPE_HPP_ */
