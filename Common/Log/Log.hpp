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
#include <map>
#include <fstream>
#include <ostream>
#include <string>

#include "../Config.hpp"

namespace Helios {

class Log {

public:

	/* Colors */
	static const char* RESET;
	static const char* RESETN;
	static const char* BLACK;
	static const char* RED;
	static const char* GREEN;
	static const char* YELLOW;
	static const char* BLUE;
	static const char* MAGENTA;
	static const char* CYAN;
	static const char* WHITE;
	static const char* BOLDBLACK;
	static const char* BOLDRED;
	static const char* BOLDGREEN;
	static const char* BOLDYELLOW;
	static const char* BOLDBLUE;
	static const char* BOLDMAGENTA;
	static const char* BOLDCYAN;
	static const char* BOLDWHITE;

	/* Enumeration of colors */
	enum Color {
		COLOR_RESET=1,
		COLOR_RESETN=2,
		COLOR_BLACK=3,
		COLOR_RED=4,
		COLOR_GREEN=5,
		COLOR_YELLOW=6,
		COLOR_BLUE=7,
		COLOR_MAGENTA=8,
		COLOR_CYAN=9,
		COLOR_WHITE=10,
		COLOR_BOLDBLACK=11,
		COLOR_BOLDRED=12,
		COLOR_BOLDGREEN=13,
		COLOR_BOLDYELLOW=14,
		COLOR_BOLDBLUE=15,
		COLOR_BOLDMAGENTA=16,
		COLOR_BOLDCYAN=17,
		COLOR_BOLDWHITE=18
	};

	/* Set output file */
	static void setOutput(const std::string& out_file);

	/* Write stuff */
	static std::ostream& msg();   /* This is a ordinary message printed into the screen */
	static std::ostream& bmsg();   /* This is a ordinary message printed into the screen */
	static std::ostream& warn();  /* A warning printed in the error channel */
	static std::ostream& error(); /* Error message */
	static std::ostream& ok();    /* O.K. message, printed on the screen */
	static std::ostream& bok();   /* O.K. message, printed on the screen (bold) */

	/* A lot of cool stuff to print on the screen / output */

	/* Indentation */
	static std::string ident(size_t n = 0);
	/* End of line */
	static const std::string endl;
	/* Reset color on output stream */
	static const std::string crst;
	 /* Print date */
	static std::string date();
	/* Print program header */
	static void header(std::ostream& out = std::cout);
	/* Get a color out stream (to standard output) */
	template<Color _color> static std::ostream& color() {
		logger.messages << color_map[_color];
		return logger.messages;
	}

	~Log();

private:

	/* There is only one logger in the program */
	static Log logger;
	Log();
	Log(const Log& log);
	Log& operator=(const Log& log);

	/* ---- Stream channels */

	/* General messages - this writes to the "console" */
	std::ostream& messages;
	/* Error channel, to standard error */
	std::ostream& oerror;
	/* Output file */
	std::ofstream output;

	/* Map of colors */
	static std::map<Log::Color,const char*> color_map;

};

} /* namespace Helios */
#endif /* LOG_HPP_ */
