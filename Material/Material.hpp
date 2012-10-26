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

#ifndef MATERIAL_HPP_
#define MATERIAL_HPP_

#include <string>
#include <vector>

#include "MaterialObject.hpp"
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

	/* Class that represents a material filling a cell */
	class Material {

	public:

		static const MaterialId NONE; /* No material */
		static const MaterialId VOID; /* Void */

		friend class MaterialFactory;
		friend std::ostream& operator<<(std::ostream& out, const Material& q);

		/* Exception */
		class BadMaterialCreation : public std::exception {
			std::string reason;
		public:
			BadMaterialCreation(const MaterialId& matid, const std::string& msg) {
				reason = "Cannot create material " + toString(matid) + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadMaterialCreation() throw() {/* */};
		};

		/* Print internal parameters of the surface */
		virtual void print(std::ostream& out) const = 0;

		/* Return the material ID. */
		const MaterialId& getUserId() const {return user_id;}
		/* Set internal / unique identifier for the material */
		void setInternalId(const InternalMaterialId& internal) {internal_id = internal;}
		/* Return the internal ID associated with this material. */
		const InternalMaterialId& getInternalId() const {return internal_id;}

		/* ---- Interaction of the material with a particle */

		/*
		 * Based on particle's energy, this functions returns the index on the energy grid with
		 * information contained on the child class.
		 */
		virtual void setEnergyIndex(const Energy& energy, EnergyIndex& index) const = 0;

		 /* Get the total cross section (using the energy index of the particle) */
		virtual double getMeanFreePath(const EnergyIndex& index) const = 0;

		/* Get reaction (based on a random generator and a energy index) */
		virtual Reaction* getReaction(const EnergyIndex& index, Random& random) const = 0;

		/* "Sample" the next reaction */
		virtual ~Material() {/* */};

	protected:

		Material(const MaterialObject* definition) : user_id(definition->getMatid()), internal_id(0) {/* */};

		/* Prevent copy */
		Material(const Material& mat);
		Material& operator= (const Material& other);

		/* Cell id choose by the user */
		MaterialId user_id;
		/* Internal identification of this surface */
		InternalMaterialId internal_id;

	};

	/* Material Factory */
	class MaterialFactory {
	public:
		/* Prevent construction or copy */
		MaterialFactory() {/* */};
		/* Create a new material */
		virtual std::vector<Material*> createMaterials(const std::vector<MaterialObject*>& definitions) const = 0;
		virtual ~MaterialFactory() {/* */}
	};

	/* Output surface information */
	std::ostream& operator<<(std::ostream& out, const Material& q);

} /* namespace Helios */
#endif /* MATERIAL_HPP_ */
