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

#ifndef MCENVIRONMENT_HPP_
#define MCENVIRONMENT_HPP_

#include <string>
#include <vector>
#include <map>

/* MC modules */
#include "McModule.hpp"

/* Common stuff */
#include "../Common/Common.hpp"

/* Parser class */
#include "../Parser/Parser.hpp"

namespace Helios {

	/* Environment class, contains all the modules that conforms the MC problem */
	class McEnvironment {

	public:

		/* Constructor from a parser */
		McEnvironment(Parser* parser);

		/*
		 * Parse file (thrown an exception if there isn't a parser) and push the definitions
		 * parsed. Thrown an exception if the parser fails.
		 */
		void parseFile(const std::string& filename);

		/*
		 * Parse files (thrown an exception if there isn't a parser) and push the definitions
		 * parsed. Thrown an exception if the parser fails.
		 */
		void parseFiles(const std::vector<std::string>& filename);

		/* Register a module factory */
		void registerFactory(ModuleFactory* factory) {
			factory_map[factory->getName()] = factory;
		}

		/* Get a module */
		template<class Module>
		Module* getModule() const;

		/* Method to setup the environment */
		void setup();

		virtual ~McEnvironment();

	private:
		/* Map between modules names and factories */
		std::map<std::string,ModuleFactory*> factory_map;

		/* Map of modules on the environment */
		std::map<std::string,McModule*> module_map;

		/* Parser pointer */
		Parser* parser;
	};

	template<class Module>
	Module* McEnvironment::getModule() const {
		/* Get the module name (all modules should have this static function) */
		std::string module = Module::name();
		/* Find on map */
		std::map<std::string,McModule*>::const_iterator it = module_map.find(module);
		/* Return module */
		if(it != module_map.end())
			return dynamic_cast<Module*>(it->second);
		else
			throw(GeneralError("Module " + module + " is not registered"));
	}

} /* namespace Helios */
#endif /* MCENVIRONMENT_HPP_ */

