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

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <string>
#include <vector>

#include "../Geometry/Geometry.hpp"

namespace Helios {

	class Parser {

	protected:
		/* Setup geometry */
		void setupGeometry(const std::vector<Geometry::SurfaceDefinition>& sur_def, const std::vector<Geometry::CellDefinition>& cell_def) const;

		/* Internal reference to the geometry of the problem */
		Geometry& geometry;

	public:

		/* Exception */
		class ParserError : public std::exception {
			std::string reason;
		public:
			ParserError(const std::string& msg) : reason(msg) {/* */}
			const char *what() const throw() {
				return reason.c_str();
			}
			virtual ~ParserError() throw() {/* */};
		};

		/* Exception */
		class KeywordParserError : public std::exception {
			/* Reason */
			std::string reason;
			/* Information to locate the *bad* keyword on the file */
			std::vector<std::string> search_keys;
		public:
			KeywordParserError(const std::string& msg, const std::vector<std::string>& search_keys) :
				               reason(msg), search_keys(search_keys) {/* */}
			const char *what() const throw() {
				return reason.c_str();
			}
			const std::vector<std::string>& getKeys() const {return search_keys;}
			virtual ~KeywordParserError() throw() {/* */};
		};

		/* Exception */
		class ParserWarning : public std::exception {
			std::string reason;
		public:
			ParserWarning(const std::string& msg) : reason(msg) {/* */}
			const char *what() const throw() {
				return reason.c_str();
			}
			virtual ~ParserWarning() throw() {/* */};
		};

		Parser(Geometry& geometry) : geometry(geometry) {/* */};

		/* Parse the geometry file, and set each surface and cell */
		virtual void parseFile(const std::string& file) const = 0;

		virtual ~Parser() {/* */};
	};

} /* namespace Helios */

#endif /* PARSER_HPP_ */
