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
#include "AceReaction/NuSampler.hpp"
#include "AceIsotopeBase.hpp"
#include "../../Environment/McModule.hpp"
#include "../../Common/Common.hpp"
#include "../Grid/MasterGrid.hpp"
#include "../Isotope.hpp"

namespace Helios {

	class AceModule: public Helios::McModule {

		/* Master grid shared by all the isotopes created on this module */
		MasterGrid* master_grid;

		/* Map of isotopes created */
		std::map<IsotopeId,AceIsotopeBase*> isotope_map;

		/* Map internal index to user index */
		std::map<IsotopeId, InternalIsotopeId> internal_isotope_map;

		/* Container of isotopes */
		std::vector<AceIsotopeBase*> isotopes;

	public:
		/* Name of the module */
		static std::string name() {return "ace-table";}

		AceModule(const std::vector<McObject*>& aceObjects, const McEnvironment* environment);

		/* Exception */
		class AceError : public std::exception {
			std::string reason;
		public:
			AceError(const IsotopeId& id, const std::string& msg) {
				reason = "Problem with isotope " + id + " : " + msg;
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
		void print(std::ostream& out) const;

		/* Get isotope map */
		std::map<IsotopeId,AceIsotopeBase*> getIsotopeMap() const {return isotope_map;}

		/* Get master grid */
		const MasterGrid* getMasterGrid() const {return master_grid;};

		virtual ~AceModule();
	};

	/* Get map of objects */
	template<>
	std::vector<AceIsotopeBase*> AceModule::getObject<AceIsotopeBase>(const UserId& id) const;

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
