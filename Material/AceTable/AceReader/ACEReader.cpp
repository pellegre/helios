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

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "ACEReader.hpp"
#include "PrintMessage.hpp"
#include "Conf.hpp"
#include "AceUtils.hpp"

/* Different ACE tables */
#include "NeutronTable.hpp"
#include "SabTable.hpp"

using namespace std;
using namespace Ace;

namespace Ace {
	/* Static instance of the reader */
	AceReader AceReader::ar;
}

AceReader::AceReader() {
	constructor_table["c"] = NeutronTable::NewTable;
	constructor_table["t"] = SabTable::NewTable;
}

AceTable* AceReader::getTable(const std::string& table_name) {
	string filename = Conf::DATAPATH + "/xsdir";
	ifstream is( filename.c_str() );
	string str="";
	if (is.is_open()) {

		while ( is.good() ) {
			getline(is,str);
			if (iStringCompare(str,"directory")) break;
		}

		while ( !is.eof() ) {
			is >> str;
			/* Obtain information for construct an ACETable Object */
			if ( str.find(table_name) != string::npos ) {
				double A; is >> A;
				string file_name; is >> file_name;
				string access_route; is >> access_route;
				int file_type; is >> file_type;
				size_t address; is >> address;
				size_t table_length; is >> table_length;
				string full_path = Conf::DATAPATH + "/" + file_name;

				string letter = table_name.substr(table_name.size() - 1);

				/* Find that letter on the table */
				table_type::iterator it_type = ar.constructor_table.find(letter);

				if(it_type != ar.constructor_table.end())
					/* Return the table */
					return ar.constructor_table[(*it_type).first](str,full_path,address);
				else {
					printMessage(PrintCodes::PrintWarning,"ACEReader::GetTable()",
								 "Letter  " + letter + " is not associated to any ACE table supported. Sorry :-( ");
					return 0;
				}
			}
		}

	} else
		throw(ACEReaderError("Could not open the file " + filename));

	is.close();
	throw(ACEReaderError("Table  " + table_name + " could not be found on xsdir. "));

}
