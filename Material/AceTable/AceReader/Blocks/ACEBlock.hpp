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

#ifndef ACEBLOCK_HPP_
#define ACEBLOCK_HPP_

#include <string>
#include <vector>
#include <fstream>
#include "../ACETable.hpp"

namespace Ace {

	/* Generic block */
	template<class T, const int JXS>
	class RawBlock : public AceTable::ACEBlock {

		std::vector<T> raw_data;

		RawBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, const int table_length, AceTable* ace_table)
		: ACEBlock(xss,ace_table) {
			/* Begin of the block */
			setBegin(xss.begin() + (jxs[JXS] - 1));
			getXSS(raw_data,table_length);
		}

	public:

		friend class NRBlock;  /* Some raw blocks are managed by NRBlock */
		friend class ANDBlock; /* Some raw blocks are managed by ANDBlock */
		friend class DLWBlock; /* Some raw blocks are managed by DLWBlock */
		friend class DLYBlock; /* Some raw blocks are managed by DLYBlock */

		/* Dump the block, on a XSS stream */
		void dump(std::ostream& xss) {putXSS(raw_data,xss);};

		int getSize() const {return raw_data.size();};

		int getType() const {return JXS;};

		/* Set the raw data of this block */
		void setData(const std::vector<T>& new_data) {
			raw_data = new_data;
		}

		const std::vector<T>& getData() const {
			return raw_data;
		}

		std::vector<T>& getData() {
			return raw_data;
		}

		std::string blockName() const {return "rawblock";};

		virtual ~RawBlock() {/* */};

	};

	class GenericBlock : public AceTable::ACEBlock {

		std::vector<std::string> raw_data;

		GenericBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, const int table_begin, AceTable* ace_table);

		void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {};
	public:

		friend class NeutronTable;

		friend class NRBlock;  /* Some raw blocks are managed by NRBlock */
		friend class ANDBlock; /* Some raw blocks are managed by ANDBlock */
		friend class DLWBlock; /* Some raw blocks are managed by DLWBlock */

		/* Dump the block, on a XSS stream */
		void dump(std::ostream& xss);

		int getSize() const {return raw_data.size();};

		int getType() const {return 0;};

		static std::string name() {return "genericblock";}

		virtual ~GenericBlock() {/* */};

	};
} /* namespace ACE */

#endif /* ACEBLOCK_HPP_ */
