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
DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ACEREADER_HPP_
#define ACEREADER_HPP_

#include "ACETable.hpp"
#include <map>
#include <string>

namespace Ace {

	class AceReader {

		/* Static instance of the reader */
		static AceReader ar;

		/* Prevent construction */
		AceReader();
		AceReader(const AceReader& ar) {/* */};

		typedef std::map<std::string,AceTable::Constructor> table_type;
		/* Map of a letter to table constructors */
		table_type constructor_table;

	public:

		/* Exception */
		class ACEReaderError : public std::exception {
			std::string reason;
		public:
			ACEReaderError(const std::string& msg) {
				reason  = "Error on Ace Reader  : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~ACEReaderError() throw() {/* */};
		};

		/* Get a ACE table object */
		static AceTable* getTable(const std::string& table_name);

		virtual ~AceReader() {/* */};
	};

} /* namespace ACE */

#endif /* ACEREADER_HPP_ */
