/*
 * Conf.hpp
 *
 *  Created on: Jun 28, 2012
 *      Author: larry
 */

#ifndef CONF_HPP_
#define CONF_HPP_

#include <string>

namespace Ace {

	class Conf {

		/* Global instance of the configuration class */
		static Conf configuration;

		/* Prevent Construction */
		Conf();
		Conf(const Conf& cp);

	public:
		/* Verbose mode flag */
		static unsigned char ShowWarnings;
		/* PATH to the xsdir file */
		static std::string DATAPATH;
		/* max characters on a line */
		static size_t MAXLINESIZE;
	};

}

#endif /* CONF_HPP_ */
