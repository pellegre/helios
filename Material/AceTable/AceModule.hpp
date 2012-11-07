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

#ifndef ACEMODULE_HPP_
#define ACEMODULE_HPP_

#include "AceReader/ReactionContainer.hpp"
#include "../../Environment/McModule.hpp"
#include "../../Common/Common.hpp"
#include "../Grid/MasterGrid.hpp"
#include "../Isotope.hpp"

namespace Helios {

	/* Isotope related to an ACE table. */
	class AceIsotope : public Isotope {

		/* Reference to a neutron table */
		Ace::ReactionContainer reactions;

		/* Atomic weight ratio */
		double aweight;
		/* Temperature at which the data were processed (in MeV) */
		double temperature;

		/* Constant reference to a CHILD grid */
		const ChildGrid* child_grid;

		/* Total cross section */
		Ace::CrossSection total_xs;
		/* Fission cross section */
		Ace::CrossSection fission_xs;
		/* Absorption cross section */
		Ace::CrossSection absorption_xs;

		void print(std::ostream& out) const;

	public:

		/* Threshold values */
		static double energy_freegas_threshold;
		static double awr_freegas_threshold;

		AceIsotope(const Ace::ReactionContainer& reactions, const ChildGrid* child_grid);

		/* Get isotope information */
		double getAwr() const {return aweight;}

		/* Get temperature (in MeVs) */
		double getTemperature() const {return temperature;}

		/* Get absorption probability */
		double getAbsorptionProb(Energy& energy) const;

		/* Get fission probability */
		double getFissionProb(Energy& energy) const;

		/* Get total cross section */
		double getTotalXs(Energy& energy) const;

		/* Fission reaction */
		void fission(Particle& particle, Random& random) const {/* */};

		/* Just one scattering reaction (from the scattering matrix) */
		void scatter(Particle& particle, Random& random) const {/* */};

		~AceIsotope() {/* */};
	};

	class AceModule: public Helios::McModule {

		/* Master grid shared by all the isotopes created on this module */
		MasterGrid* master_grid;

		/* Map of isotopes created */
		std::map<IsotopeId,AceIsotope*> isotope_map;

		/* Map internal index to user index */
		std::map<IsotopeId, InternalIsotopeId> internal_isotope_map;

		/* Container of isotopes */
		std::vector<AceIsotope*> isotopes;

	public:
		/* Name of the module */
		static std::string name() {return "ace-table";}

		AceModule(const std::vector<McObject*>& aceObjects, const McEnvironment* environment);

		/* Exception */
		class AceError : public std::exception {
			std::string reason;
		public:
			AceError(const IsotopeId& id, const std::string& msg) {
				reason = "Cannot access to isotope " + id + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~AceError() throw() {/* */};
		};


		/* Get references to objects from an id */
		template<class Object>
		std::vector<Object*> getObject(const UserId& id) const;

		/* Print descriptions of each isotope and grid information */
		void printIsotopes(std::ostream& out) const;

		/* Get isotope map */
		std::map<IsotopeId,AceIsotope*> getIsotopeMap() const {return isotope_map;}

		/* Get master grid */
		const MasterGrid* getMasterGrid() const {return master_grid;};

		virtual ~AceModule();
	};

	/* Get map of objects */
	template<>
	std::vector<AceIsotope*> AceModule::getObject<AceIsotope>(const UserId& id) const;

	/* Class to define an Ace Object (which will produce an AceIsotope as a product) */
	class AceObject : public McObject {
		std::string table_name;
	public:
		friend class AceModule;
		/* Different type of geometric entities */
		AceObject(const std::string& table_name) : McObject(AceModule::name(),"ace-isotope"), table_name(table_name) {/* */};
		virtual ~AceObject(){/* */};
	};

	class McEnvironment;

	/* Material Factory */
	class AceFactory : public ModuleFactory {
	public:
		/* Prevent construction or copy */
		AceFactory(McEnvironment* environment) : ModuleFactory(AceModule::name(),environment) {/* */};
		/* Create a new material */
		McModule* create(const std::vector<McObject*>& objects) const {
			return new AceModule(objects,getEnvironment());
		}
		virtual ~AceFactory() {/* */}
	};

} /* namespace Helios */
#endif /* ACEMODULE_HPP_ */
