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

#ifndef SETTINGSOBJECT_HPP_
#define SETTINGSOBJECT_HPP_

#include <string>
#include <map>

#include "../McModule.hpp"
#include "../../Common/Common.hpp"

namespace Helios {

	/* Generic and base class to define all source related objects */
	class SettingsObject : public McObject {
		/* ID of this setting */
		std::string setting_name;
		/* Map of names and values of this setting */
		std::map<std::string, std::string> settings;
	public:
		/* Set settings */
		SettingsObject(const MaterialId& setting_name, const std::map<std::string, std::string>& settings) :
			McObject("settings","setting"), setting_name(setting_name), settings(settings) {/* */};

		SettingsObject(const MaterialId& setting_name, const std::string& value) :
			McObject("settings","setting"), setting_name(setting_name) {
			settings["value"] = value;
		};

		virtual ~SettingsObject(){/* */};

		std::string getSettingName() const {
			return setting_name;
		}

		std::map<std::string, std::string> getSettings() const {
			return settings;
		}
	};
}


#endif /* SETTINGSOBJECT_HPP_ */
