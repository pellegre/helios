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

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <map>
#include <set>
#include <string>
#include <boost/lexical_cast.hpp>

#include "../McModule.hpp"
#include "SettingsObject.hpp"

namespace Helios {

	/* Base class for a setting */
	class Setting {
		/* Setting name */
		UserId setting_name;
		/* Map of keys and values of each internal setting */
		std::map<std::string, std::string> settings;

		/* Print settings */
		void print(std::ostream& out) const;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Setting& q);
	public:
		/* Object name */
		static std::string name() {return "setting";}

		Setting(const SettingsObject* settingsObject) :
			setting_name(settingsObject->getSettingName()), settings(settingsObject->getSettings()) {/* */}

		/* Get setting as some type using a key */
		template<class Type>
		Type get(const std::string& key) const;

		/* Get information of the setting */
		std::string getName() const {return setting_name;}

		~Setting() {/* */}
	};

	/* Output surface information */
	std::ostream& operator<<(std::ostream& out, const Setting& q);

	class Settings: public Helios::McModule {
		/* Map of settings name and objects */
		std::map<UserId, Setting*> settings_map;
		/* Registered settings (and valid keys) */
		std::map<UserId,std::set<std::string> > valid_settings;

		/* Set valid settings */
		void setValidSettings();
		/* Set single value */
		void setSingleValue(const std::string& setting);

		/* Push setting (checking validity) */
		void pushSetting(const SettingsObject* object);

	public:

		/* ---- Module stuff */

		/* Name of this module */
		static std::string name() {return "settings"; }

		Settings(const std::vector<McObject*>& setDefinitions, const McEnvironment* environment);

		/* Exception */
		class SettingsError : public std::exception {
			std::string reason;
		public:
			SettingsError(const std::string& msg) {
				reason = msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~SettingsError() throw() {/* */};
		};

		/* ---- Get information */

		/* Get setting */
		Setting* getSetting(const UserId& name) const;

		/* Get object */
		template<class Object>
		std::vector<Object*> getObject(const UserId& id) const;

		/* Print settings */
		void printSettings(std::ostream& out) const;

		virtual ~Settings();
	};

	/* Get object */
	template<class Object>
	std::vector<Object*> Settings::getObject(const UserId& id) const {
		/* Get pointer */
		Object* object = dynamic_cast<Object*>(getSetting(id));
		/* Construct container */
		std::vector<Object*> objContainer;
		objContainer.push_back(object);
		/* Return object */
		return objContainer;
	}

	class McEnvironment;

	/* Material Factory */
	class SettingsFactory : public ModuleFactory {
	public:
		/* Prevent construction or copy */
		SettingsFactory(McEnvironment* environment) : ModuleFactory(Settings::name(),environment) {/* */};
		/* Create a new material */
		McModule* create(const std::vector<McObject*>& objects) const {
			return new Settings(objects,getEnvironment());
		}
		virtual ~SettingsFactory() {/* */}
	};


	/* Get internal value of a setting */
	template<class Type>
	Type Setting::get(const std::string& key) const {
		/* Get setting */
		std::map<std::string, std::string>::const_iterator it = settings.find(key);
		/* Check if key exists */
		if(it != settings.end())
			/* Cast value to the type */
			return boost::lexical_cast<Type>((*it).second);
		else
			/* Error */
			throw(Settings::SettingsError("Key " + toString(key) + " does not exist on setting " + setting_name));
	}


} /* namespace Helios */
#endif /* SETTINGS_HPP_ */
