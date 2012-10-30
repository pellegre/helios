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
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <cstdlib>

#include "ACETable.hpp"
#include "Conf.hpp"
#include "PrintMessage.hpp"
#include "Utils.hpp"

using namespace std;
using namespace ACE;

ACETable::ACETable(const string& _table_name, const string& full_path, size_t address, int last_table) : table_name(_table_name){
	/* File */
	ifstream is(full_path.c_str());
	size_t line_number = 0;
	/* Generic dummy data */
	string line;

	if ( is.is_open() ) {

		while( !is.eof() && (line_number < address-1) ) {
			is.ignore(Conf::MAXLINESIZE,'\n');
			line_number++;
		}

		if( is.eof() ) {
			printMessage(PrintCodes::PrintError,"ACETable::ACETable()",
					     "Could not find the table name on file " + full_path);
			exit(1);
		}

		/* Verify the table name */
		is >> line;
		if (line.find(table_name) == string::npos) {
			printMessage(PrintCodes::PrintError,"ACETable::ACETable()",
					     "The address supply in xsdir doesn't match this isotope name");
			exit(1);
		}

		/* Get atomic weight */
		is >> aweight;
		/* Get temperature, in MeVs */
		is >> temperature;
		/* Date */
		is >> date;

		is.ignore(Conf::MAXLINESIZE,'\n');

		/* Get the comment of the table */
		getline(is,comment);

		/* Fill miscellaneous information  */
		for (int i = 0 ; i < iz_size ; i++) {
			is >> iz[i]; is >> aw[i];
		}

		/* Get the NXS array */
		for (int i = 0 ; i < nxs_size ; i++) is >> nxs[i];

		/* Get the JXS array */
		for (int i = 0 ; i < jxs_size ; i++) is >> jxs[i];

		if(!last_table) {

			/* Get the XSS array */
			xss.resize(nxs[0]);
			for (int i = 0 ; i < nxs[0] ; i++) is >> xss[i];

		} else {
			/* 741158 */
			int itable = last_table;
			int old_ptr = jxs[itable]; /* Before */
			while(!old_ptr) {
				itable++;
				old_ptr = jxs[itable];
			}
			/* Get all shifts */
			vector<int> shifts;
			for(int ipos = 0; ipos < ACETable::jxs_size ; ipos++) {
				int ptr = jxs[ipos];
				if(ptr && (ipos != itable) && (ipos != 20 ) && (ptr >= old_ptr) )
					/*
					 * ipos should be != than 20 because the FIS block is inside the SIG block :-p.
					 * Is not like the FIS block is an independent one, is inside another. Damn pricks.
					 */
					shifts.push_back(ptr);
			}

			/* The less shift should be  size of the table we should read */
			vector<int>::iterator it_min = min_element(shifts.begin(),shifts.end());

			int length = (*it_min);

			if(shifts.size() == 1) /* This is the last block on the table */
				length++; /* The END = JXS(22) pointer points to the last word of the table */

			/* Get the XSS array */
			xss.resize(length - 1);
			for (int i = 0 ; i < length - 1 ; i++) is >> xss[i];

			/* We should read the rest of the data (as strings) */
			string str;
			while ( is.good() ) {
				getline(is,str);
				if(str.find("c") != string::npos) break;
				rest_data.push_back(str);
			}
		}

	} else {
		printMessage(PrintCodes::PrintError,"ACETable::ACETable()",
				     "Could not open the file " + full_path);
		exit(1);
	}

	is.close();
}

/* Print general information of the library */
void ACETable::printInformation(std::ostream& out) const {
	out << "[+]" << comment << endl;
	out << "[@] Table name    : " << table_name << endl;
	out << "[@] Atomic weight : " << aweight << endl;
	out << "[@] Temperature   : " << temperature << " MeV " << endl;
	out << "[@] Date          : " << date << endl;
	printTableInfo(out);
}

void ACETable::updateData() {
	vector<ACEBlock*>::reverse_iterator it_block;

	for(it_block = blocks.rbegin() ; it_block != blocks.rend() ; it_block++)
		(*it_block)->updateData();
}

void ACETable::updateBlocks() {
	updateData();

	int jxs_new[jxs_size];

	memcpy(jxs_new,jxs,jxs_size * sizeof(int));

	vector<ACEBlock*>::iterator it_block;

	for(it_block = blocks.begin() ; it_block != blocks.end() ; it_block++)
		(*it_block)->updatePointers(nxs,jxs,jxs_new);

	memcpy(jxs,jxs_new,jxs_size * sizeof(int));

	updateData();
}

void ACETable::dump(std::ostream& out) {
	out.setf(ios::uppercase);

	/* First line */
	out.width(10);
	out << table_name;
	out.width(12);
	out << fixed << aweight;
	out.width(12);
	out << scientific << setprecision(5) << temperature;
	out.width(11);
	out << date;
	out << endl;

	/* Second line (comment) */
	out << comment << endl;

	/* IZ and AW arrays */
	for(int i = 0 ; i < iz_size ; i++) {
		out.width(7);
		out << dec << iz[i];
		out.width(11);
		out << fixed << std::setprecision(6) << aw[i];
		if(i%4 == 3) out << endl;
	}

	updateBlocks();

	/* NXS array */
	for (int i = 0 ; i < nxs_size ; i++) {
		out.width(9);
		out << nxs[i];
		if(i%8 == 7) out << endl;
	}

	/* JXS array */
	for (int i = 0 ; i < jxs_size ; i++) {
		out.width(9);
		out << jxs[i];
		if(i%8 == 7) out << endl;
	}

	/* Now, dump each block */
	vector<ACEBlock*>::iterator it_block;

	for(it_block = blocks.begin() ; it_block != blocks.end() ; it_block++)
		(*it_block)->dump(out);

	vector<string>::iterator it_rest;

	for(it_rest = rest_data.begin() ; it_rest != rest_data.end() ; it_rest++) {
		if ((*it_rest).size() > 0)
			out << (*it_rest) << endl;
	}
}

ACETable::~ACETable() {
	vector<ACEBlock*>::iterator it;
	for(it = blocks.begin() ; it != blocks.end() ; it++)
		delete (*it);
};

/* Update pointers on the ACE table according  to data on this block */
void ACETable::ACEBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	shiftJXSArray(jxs_old,jxs_new,getType(),getSize());
};

ACETable::ACEBlock::~ACEBlock() {/* */};
