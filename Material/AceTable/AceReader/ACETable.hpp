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

#ifndef ACETABLE_HPP_
#define ACETABLE_HPP_

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>

namespace Ace {

	class AceTable {

	public:
		/* Some size */
		static const int nxs_size=16;
		static const int jxs_size=32;
		static const int iz_size=16;
		static const int aw_size=16;

		class ACEBlock {

			/* Iterator to the beginning of the XSS array */
			std::vector<double>::const_iterator it;

		protected:

			/* Some size */
			static const int nxs_size=16;
			static const int jxs_size=32;

			/* Update internal data of the block (should be called BEFORE UpdatePointers) */
			virtual void updateData() {/* */};

			/* Update pointers on the ACE table according  to data on this block */
			virtual void updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const;

			/* Get a number from the XSS array (only used for XSS array) */
			template<class T>
			void getXSS(T& number) {
				number = T((*it));
				++it;
			}
			template<class Seq>
			void getXSS(Seq& v, size_t table_length) {
				v.resize(table_length);
				for(size_t i = 0 ; i < v.size() ; i++)
					getXSS(v[i]);
			}

			/* Set the start point to parse the XSS array */
			void setBegin(std::vector<double>::const_iterator begin) {
				it = begin;
			}

			/* Get the position on the XSS array */
			std::vector<double>::const_iterator getPosition() const {
				return it;
			}

			/* Pointer to the ACE table that contains this block */
			AceTable* ace_table;

			ACEBlock(const std::vector<double>& xss, AceTable* ace_table) : it(xss.begin()), ace_table(ace_table) {/* */};
			ACEBlock(std::vector<double>::const_iterator _it, AceTable* ace_table) : it(_it), ace_table(ace_table) {/* */};

		public:

			friend class AceTable; /* Because a table contains a lot of blocks (with up-casting) */

			/* Dump the block, on a XSS stream */
			virtual void dump(std::ostream& xss) = 0;

			virtual int getSize() const = 0;

			virtual int getType() const = 0;

			virtual std::string blockName() const = 0;

			virtual ~ACEBlock();

		};

	protected:
		/* ---------- Construction of tables */

		/* ACE Table constructor function */
		typedef AceTable(*(*Constructor)(const std::string&, const std::string&, size_t));

		/* ---------- Miscellaneous Variables on Data Tables */

		/* Name of the table */
		std::string table_name;
		/* Atomic weight ratio */
		double aweight;
		/* Temperature at which the data were processed (in MeV) */
		double temperature;
		/* Date when data were processed */
		std::string date;
		/* Comment */
		std::string comment;

		/* Container of blocks */
		std::vector<ACEBlock*> blocks;

		/* ---------- Arrays on Data Tables */

		/* Pairs of ZZZAAAs and atomic weight ratios (most of the time this is ignored) */
		int iz[iz_size];
		double aw[aw_size];

		/* Pointer and counter tables */
		int nxs[nxs_size];
		int jxs[jxs_size];

		/* When the table is read from a file, we save here all the xs table */
		std::vector<double> xss;
		std::vector<std::string> rest_data;

		/* ---------- Virtual abstract functions */

		/* Print general information of the library */
		virtual void printTableInfo(std::ostream& out = std::cout) const = 0;

		virtual void updateData();

		/* Constructor, from full path and address on that file */
		AceTable(const std::string& _table_name, const std::string& full_path, size_t address, int last_table = 0);

	public:

		/* Exception */
		class AceTableError : public std::exception {
			std::string reason;
		public:
			AceTableError(const AceTable* ace_table, const std::string& msg) {
				reason  = "Error on Ace Table " + ace_table->getName() + " : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~AceTableError() throw() {/* */};
		};

		/* Friendly reader */
		friend class AceReader;

		/* Write the cross section table */
		void dump(std::ostream& out = std::cout);

		/* Update pointers and data in ACE blocks */
		void updateBlocks();

		/* Print general information of the library */
		void printInformation(std::ostream& out = std::cout) const;

		/* Get name of the table */
		std::string getName() const {return table_name;}

		/* Get atomic weight ratio */
		double getAtomicRatio() const {return aweight;}

		/* Get temperature (this is in Mevs)*/
		double getTemperature() const {return temperature;}

		/* Get date of the table */
		std::string getDate() const {return date;}

		/* Get comment */
		std::string getComment() const {return comment;}

		virtual ~AceTable();
	};

} /* namespace ACE */

#endif /* ACETABLE_HPP_ */
