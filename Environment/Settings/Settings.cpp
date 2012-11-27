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

#include "Settings.hpp"

using namespace std;

namespace Helios {

void Settings::setSingleValue(const std::string& setting) {
	valid_settings[setting].insert("value");
}

void Settings::setValidSettings() {
	setSingleValue("max_source_samples");
	setSingleValue("max_rng_per_history");
	setSingleValue("xs_data");
	setSingleValue("multithread");
	setSingleValue("seed");
	setSingleValue("energy_freegas_threshold");
	setSingleValue("awr_freegas_threshold");

	/* KEFF simulation data */
	valid_settings["criticality"].insert("batches");
	valid_settings["criticality"].insert("inactive");
	valid_settings["criticality"].insert("particles");
}

Settings::Settings(const std::vector<McObject*>& setDefinitions, const McEnvironment* environment)
	: McModule(name(),environment) {

	/* Set valid settings */
	setValidSettings();

	/* Loop over definitions and set each setting */
	for(vector<McObject*>::const_iterator it = setDefinitions.begin() ; it != setDefinitions.end() ; ++it) {
		/* Add settings */
		SettingsObject* newObject = static_cast<SettingsObject*>(*it);
		/* Push settings */
		pushSetting(newObject);
	}
}

void Settings::pushSetting(const SettingsObject* object) {
	/* Get name */
	string name = object->getSettingName();
	/* Check validity */
	map<UserId,set<string> >::const_iterator it = valid_settings.find(name);
	if(it != valid_settings.end()) {
		/* Get set of keys */
		const set<string>& keys = (*it).second;
		/* Get user keys */
		const map<string,string>& user_keys = object->getSettings();
		/* Check each keys */
		for(map<string,string>::const_iterator it_keys = user_keys.begin() ; it_keys != user_keys.end() ; ++it_keys)
			if(keys.find((*it_keys).first) == keys.end())
				throw(Settings::SettingsError("Key " + (*it_keys).first + " on setting " + name + " is not valid"));
	} else
		throw(Settings::SettingsError("Setting " + name + " not recognized"));

	/* Create setting */
	settings_map[name] = new Setting(object);
}

/* Print settings */
void Settings::printSettings(std::ostream& out) const {
	out << "  - Settings " << endl;
	for(std::map<UserId, Setting*>::const_iterator it = settings_map.begin() ; it != settings_map.end() ; ++it)
		out << "   " << *(*it).second << endl;
}

Setting* Settings::getSetting(const UserId& name) const {
	std::map<UserId,Setting*>::const_iterator set = settings_map.find(name);
	if(set == settings_map.end())
		throw SettingsError("Setting " + name + " does not exist");
	else
		return (*set).second;
}

Settings::~Settings() {
	/* Delete settings */
	for(map<UserId, Setting*>::const_iterator it = settings_map.begin() ; it != settings_map.end() ; ++it)
		delete (*it).second;
}

void Setting::print(std::ostream& out) const {
	/* Print information of the setting */
	out << Log::ident(1) << setw(35) << setting_name;
	/* Print values */
	out << " ( ";
	for(map<std::string, std::string>::const_iterator it = settings.begin() ; it != settings.end() ; ++it)
		out << (*it).first << " = " << (*it).second << " ";
	out << ")";
}

std::ostream& operator<<(std::ostream& out, const Setting& q) {
	q.print(out);
	return out;
}

} /* namespace Helios */
