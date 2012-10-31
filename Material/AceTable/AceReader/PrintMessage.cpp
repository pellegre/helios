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

#include "PrintMessage.hpp"
#include "../../../Common/Common.hpp"

using namespace std;
using namespace ACE;
using namespace Helios;

namespace ACE {

	namespace Conf {
		extern /* Verbose mode flag */
		unsigned char ShowWarnings;
	}
}

void ACE::printMessage(int code, const string& routine, const string& message) {
	string code_str;

	switch(code) {
		/* Just print some message */
		case PrintCodes::PrintMessage :
			code_str = "Ace Module message ";
			break;

	    /* Print a warning */
		case PrintCodes::PrintWarning :
			code_str = "Ace Module warning ";
			break;

		/* Print the error message */
		case PrintCodes::PrintError :
			code_str = "Ace Module error ";
			break;

		default:
			code_str = "";
			break;

	}

	/* Print String */
	string ret_str = code_str + " from " + routine + " : " + message;

	if(code == PrintCodes::PrintMessage) {
		Log::msg() << ret_str << Log::endl;
	}
	else if (code == PrintCodes::PrintWarning) {
		if(Conf::ShowWarnings)
			Log::warn() << ret_str << Log::endl;
	}
	else {
		Log::error() << ret_str << Log::endl;
	}

}



