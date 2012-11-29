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
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <ctime>
#include <tbb/tbb_stddef.h>
#include <boost/version.hpp>

#include "Log.hpp"
#include "../Config.hpp"

using namespace std;

namespace Helios {

Log Log::logger;

const char* Log::RESET = "\033[0m";
const char* Log::RESETN = "\033[0m\n";
const char* Log::BLACK = "\033[30m";
const char* Log::RED = "\033[31m";
const char* Log::GREEN = "\033[32m";
const char* Log::YELLOW = "\033[33m";
const char* Log::BLUE = "\033[34m";
const char* Log::MAGENTA = "\033[35m";
const char* Log::CYAN = "\033[36m";
const char* Log::WHITE = "\033[37m";
const char* Log::BOLDBLACK = "\033[1m\033[30m";
const char* Log::BOLDRED = "\033[1m\033[31m";
const char* Log::BOLDGREEN = "\033[1m\033[32m";
const char* Log::BOLDYELLOW = "\033[1m\033[33m";
const char* Log::BOLDBLUE = "\033[1m\033[34m";
const char* Log::BOLDMAGENTA = "\033[1m\033[35m";
const char* Log::BOLDCYAN = "\033[1m\033[36m";
const char* Log::BOLDWHITE = "\033[1m\033[37m";

const std::string Log::endl = RESETN;
const std::string Log::crst = RESET;

/* Initialize color map */
map<Log::Color,const char*> initColorMap() {
	map<Log::Color,const char*> m;
	m[Log::COLOR_RESET] = Log::RESET;
	m[Log::COLOR_RESETN] = Log::RESETN;
	m[Log::COLOR_BLACK] = Log::BLACK;
	m[Log::COLOR_RED] = Log::RED;
	m[Log::COLOR_GREEN] = Log::GREEN;
	m[Log::COLOR_YELLOW] = Log::YELLOW;
	m[Log::COLOR_BLUE] = Log::BLUE;
	m[Log::COLOR_MAGENTA] = Log::MAGENTA;
	m[Log::COLOR_CYAN] = Log::CYAN;
	m[Log::COLOR_WHITE] = Log::WHITE;
	m[Log::COLOR_BOLDBLACK] = Log::BOLDBLACK;
	m[Log::COLOR_BOLDRED] = Log::BOLDRED;
	m[Log::COLOR_BOLDGREEN] = Log::BOLDGREEN;
	m[Log::COLOR_BOLDYELLOW] = Log::BOLDYELLOW;
	m[Log::COLOR_BOLDBLUE] = Log::BOLDBLUE;
	m[Log::COLOR_BOLDMAGENTA] = Log::BOLDMAGENTA;
	m[Log::COLOR_BOLDCYAN] = Log::BOLDCYAN;
	m[Log::COLOR_BOLDWHITE] = Log::BOLDWHITE;
	return m;
}

map<Log::Color,const char*> Log::color_map = initColorMap();

Log::Log() : messages(cout), oerror(cerr) {/* */}

/* Set output file */
void Log::setOutput(const std::string& out_file) {
	/* Open the output file */
	logger.output.open(out_file.c_str());
	if (!logger.output.is_open()) {
        cerr << "[E] Error opening the output file (" + out_file + ") : " << strerror(errno) << endl;
        exit(1);
    }
}

std::string Log::ident(size_t n) {
	string idt = "  ";
	for(size_t i = 0 ; i < n ; i++) idt += idt;
	return idt;
}

std::ostream& Log::msg() {
	logger.messages << ident(0);
	return logger.messages;
}

std::ostream& Log::bmsg() {
	logger.messages << BOLDWHITE << ident(0);
	return logger.messages;
}

std::ostream& Log::warn() {
	logger.oerror << BOLDYELLOW << ident(0) << "[W] " << RESET << YELLOW;
	return logger.oerror;
}

std::ostream& Log::error() {
	logger.oerror << BOLDRED << ident(0) << "[E] " << RESET << RED;
	return logger.oerror;
}

std::ostream& Log::ok() {
	logger.messages << ident(0) << RESET << CYAN;
	return logger.messages;
}

std::ostream& Log::bok() {
	logger.messages << ident(0) << RESET << BOLDCYAN;
	return logger.messages;
}

std::string Log::date()  {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime (&rawtime);
	return string(asctime(timeinfo));
}

void Log::setRank(int rank) {
	if(rank != 0) {
		std::cout.setstate(std::ios::failbit);
		std::cerr.setstate(std::ios::failbit);
	}
}

void Log::header(std::ostream& out) {
	logger.messages << endl;
	/* Print header */
	logger.messages << BOLDBLUE << ident(0) <<"   / / / /__  / (_)___  _____  __    __"    << endl;
	logger.messages << BOLDBLUE << ident(0) <<"  / /_/ / _ \\/ / / __ \\/ ___/_/ /___/ /_" << endl;
	logger.messages << BOLDBLUE << ident(0) <<" / __  /  __/ / / /_/ (__  )_  __/_  __/"   << endl;
	logger.messages << BOLDBLUE << ident(0) <<"/_/ /_/\\___/_/_/\\____/____/ /_/   /_/"    << crst << endl << endl;
	/* General information */
	logger.messages << ident(0) << BOLDWHITE << "A Continuous-energy Monte Carlo Reactor Physics Code" << endl << endl;
	logger.messages << ident(0) << " - Version    : " << PROJECT_VERSION << endl;
	logger.messages << ident(0) << " - Contact    : Esteban Pellegrino (pellegre@ib.cnea.gov.ar) " << endl;
	/* Build related stuff */
	logger.messages << ident(0) << " - Compiler   : " << COMPILER_NAME << endl;
	logger.messages << ident(0) << " - Build type : " << BUILD_TYPE << endl;
	logger.messages << ident(0) << " - Build date : " << COMPILATION_DATE << " (commit " << GIT_SHA1 << ")" << endl;
	logger.messages << ident(0) << " - Boost      : Version " << BOOST_LIB_VERSION << endl;
	logger.messages << ident(0) << " - Intel TBB  : Version " << TBB_VERSION_MAJOR << "." << TBB_VERSION_MINOR << endl;
	/* Print when the calculation began */
	logger.messages << endl << BOLDWHITE << ident(0) << "Begin calculation on " << date() << endl;
}

Log::~Log() {
	output.close();
}

} /* namespace Helios */
