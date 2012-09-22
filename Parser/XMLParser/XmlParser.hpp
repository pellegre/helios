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

#ifndef XMLPARSER_HPP_
#define XMLPARSER_HPP_

#include <vector>
#include <string>

#include "tinyxml.h"
#include "../Parser.hpp"

namespace Helios {

	class XmlParser : public Parser {

		/* Prevent creation */
		XmlParser();
		/* Prevent copy */
		XmlParser(const XmlParser& p);
		XmlParser& operator=(const XmlParser& p);

		/* Process root node */
		void rootNode(TiXmlNode* pParent) const;

		/* ---- Process nodes */
		static void geoNode(TiXmlNode* pParent);

		/* Map of functions for each root node */
		typedef void (*NodeParser)(TiXmlNode* node);
		std::map<std::string,NodeParser> root_map;

	public:

		/* Access the parser */
		static Parser& access();

		/* Parse a file */
		void parseFile(const std::string& file) const;

		virtual ~XmlParser() {/* */};
	};

} /* namespace Helios */

#endif /* XMLPARSER_H_ */
