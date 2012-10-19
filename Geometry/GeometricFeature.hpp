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

#ifndef GEOMETRICFEATURE_HPP_
#define GEOMETRICFEATURE_HPP_

#include <vector>
#include <map>
#include <string>

#include "Cell.hpp"
#include "Surface.hpp"
#include "../Common/Common.hpp"
#include "GeometryObject.hpp"

namespace Helios {

	/*
	 * A geometric feature is a collection of geometry entities that conform a complex
	 * object. For example, a pin, pin ring or a lattice.
	 */
	class GeometricFeature {

	protected:

		/* Name of the feature */
		std::string feature;

	public:

		static std::string name() {return "feature";}

		class Definition : public GeometryObject {

		protected:
			/* The universe where this feature will be constructed */
			UniverseId userFeatureId;
			/* Name of the feature */
			std::string feature;
		public:
			Definition(const std::string& feature, const UniverseId& userFeatureId) :
				GeometryObject(GeometricFeature::name()), feature(feature), userFeatureId(userFeatureId) {/* */}
			std::string getFeature() const {
				return feature;
			}
			UniverseId getUserFeatureId() const {
				return userFeatureId;
			}
			virtual ~Definition() {/* */}
		};

		GeometricFeature(const Definition* definition) :
						feature(definition->getFeature()) {/* */};

		/*
		 * Create the the feature and add new cells/surfaces into the containers.
		 * Returns the number of the new range of max user IDs.
		 */
		virtual void createFeature(const Definition* featureDefinition,
								   std::vector<Surface::Definition*>& surfaceDefinition,
								   std::vector<Cell::Definition*>& cellDefinition) const = 0;

		virtual ~GeometricFeature() {/* */};
	};

	/* Lattice factory class */
	class Lattice : public GeometricFeature {

	public:

		class Definition : public GeometricFeature::Definition {
			std::string type;
			std::vector<unsigned int> dimension;
			std::vector<double> pitch;
			std::vector<UniverseId> universes;
		public:

			Definition(const UniverseId& userLatticeId, const std::string& type, const std::vector<unsigned int>& dimension,
					   const std::vector<double>& pitch, const std::vector<UniverseId>& universes) :
					   GeometricFeature::Definition("lattice",userLatticeId), type(type), dimension(dimension),
					   pitch(pitch), universes(universes) {/* */}

			std::vector<unsigned int> getDimension() const {
				return dimension;
			}

			std::string getType() const {
				return type;
			}

			std::vector<UniverseId> getUniverses() const {
				return universes;
			}

			std::vector<double> getWidth() const {
				return pitch;
			}

			~Definition() {/* */}
		};

		typedef void(*Constructor)(const Lattice::Definition& new_lat,
								   std::vector<Surface::Definition*>& sur_def,
								   std::vector<Cell::Definition*>& cell_def);

		/* Constructor with current surfaces and cells on the geometry */
		Lattice(const GeometricFeature::Definition* definition);

		void createFeature(const GeometricFeature::Definition* featureDefinition,
						   std::vector<Surface::Definition*>& surfaceDefinition,
						   std::vector<Cell::Definition*>& cellDefinition) const;

		virtual ~Lattice() {/* */}

	private:

		std::string type;
		std::vector<unsigned int> dimension;
		std::vector<double> pitch;
		std::vector<UniverseId> universes;

		/* Map of lattices types and constructors */
		static std::map<std::string, Constructor> constructor_table;

	};

	class FeatureFactory {

		/* Static instance of the factory */
		static FeatureFactory factory;

		/* Prevent construction or copy */
		FeatureFactory() {/* */};
		FeatureFactory& operator= (const FeatureFactory& other);
		FeatureFactory(const FeatureFactory&);
		virtual ~FeatureFactory() {/* */}

	public:
		/* Access the factory, reference to the static singleton */
		static FeatureFactory& access() {return factory;}

		/* Create a new surface */
		GeometricFeature* createFeature(const GeometricFeature::Definition* definition) const {
			if(definition->getFeature() == "lattice")
				return new Lattice(definition);
			return 0;
		}

	};
} /* namespace Helios */
#endif /* GEOMETRICFEATURE_HPP_ */
