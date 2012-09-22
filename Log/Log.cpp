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

#include "Log.hpp"

#define RESET   "\033[0m"
#define RESETN  "\033[0m\n"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

using namespace std;

namespace Helios {

Log Log::logger;

const std::string Log::endl = RESETN;

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
	logger.messages << ident(1);
	return logger.messages;
}

std::ostream& Log::warn() {
	logger.oerror << BOLDYELLOW << "[W] " << RESET << YELLOW;
	return logger.oerror;
}

std::ostream& Log::error() {
	logger.oerror << BOLDRED << "[E] " << RESET << RED;
	return logger.oerror;
}

std::ostream& Log::ok() {
	logger.messages << BOLDCYAN << "[@] " << RESET << CYAN;
	return logger.messages;
}

Log::~Log() {
	output.close();
}

} /* namespace Helios */
