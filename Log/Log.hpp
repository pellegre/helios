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

#ifndef LOG_HPP_
#define LOG_HPP_

#include <iostream>
#include <fstream>
#include <ostream>
#include <string>

namespace Helios {

class Log {
	/* There is only one logger in the program */
	static Log logger;
	Log();
	Log(const Log& log);
	Log& operator=(const Log& log);

	/* ---- Stream channels */

	/* General messages - this does to the "console" */
	std::ostream& messages;
	/* Error channel, to standard error */
	std::ostream& oerror;
	/* Output file */
	std::ofstream output;

public:

	/* Set output file */
	static void setOutput(const std::string& out_file);

	/* Write stuff */
	static std::ostream& msg();   /* This is a ordinary message printed into the screen */
	static std::ostream& warn();  /* A warning printed in the error channel */
	static std::ostream& error(); /* Error message */
	static std::ostream& ok();    /* O.K. message, printed on the screen */

	/* A lot of cool stuff to print on the screen / output */

	static std::string ident(size_t n = 0);      /* Indentation */
	static const std::string endl;               /* End of line */

	~Log();
};

} /* namespace Helios */
#endif /* LOG_HPP_ */
