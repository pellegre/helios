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
#include <iostream>
#include <string>
#include <vector>

#include "../../Parser/ParserTypes.hpp"
#include "../../Log/Log.hpp"
#include "../../Geometry/Geometry.hpp"
#include "pngwriter.hpp"

using namespace std;
using namespace Helios;

static double colorFromCell(const InternalCellId& cell_id, const InternalCellId& max_id) {
	return (double)cell_id / (double)max_id;
}

void plot(const Helios::Geometry& geo, double xmin, double xmax, double ymin, double ymax, const std::string& filename) {
	static int pixel = 500;
	pngwriter png(pixel,pixel,1.0,filename.c_str());
	/* Deltas */
	double deltax = (xmax - xmin) / (double)(pixel);
	double deltay = (ymax - ymin) / (double)(pixel);
	/* Number of cells */
	size_t max_id = geo.getCellNumber();
	const Cell* find_cell = geo.findCell(Coordinate(0.0,0.0,0.0));
	InternalCellId old_cell_id = 0;
	if(find_cell)
		old_cell_id= find_cell->getInternalId();
	/* Loop over pixels */
	for(int i = 0 ; i < pixel ; ++i) {
		for(int j = 0 ; j < pixel ; ++j) {
			double x = xmin + (double)i * deltax;
			double y = ymin + (double)j * deltay;
			Coordinate point(Coordinate(x,y,0.0));
			const Cell* check_old = find_cell->findCell(point);
			/* Get cell ID */
			if(check_old)
				find_cell = check_old;
			else
				find_cell = geo.findCell(point);
			InternalCellId new_cell_id = 0;
			if(find_cell)
				new_cell_id = find_cell->getInternalId();
			if(new_cell_id != old_cell_id || !find_cell) {
				png.plot(i,j,0.0,0.0,0.0);
			} else {
				double color = colorFromCell(new_cell_id,max_id);
				png.plotHSV(i,j,color,1.0,1.0);
			}
			old_cell_id = new_cell_id;
		}
	}

	/* Mark the "black" line on the other direction */
	find_cell = geo.findCell(Coordinate(0.0,0.0,0.0));
	if(find_cell)
		old_cell_id = find_cell->getInternalId();
	else
		old_cell_id = 0;

	for(int j = 0 ; j < pixel ; ++j) {
		for(int i = 0 ; i < pixel ; ++i) {
			double x = xmin + (double)i * deltax;
			double y = ymin + (double)j * deltay;
			/* Get cell ID */
			find_cell = geo.findCell(Coordinate(x,y,0.0));
			InternalCellId new_cell_id = 0;
			if(find_cell)
				new_cell_id = find_cell->getInternalId();
			if(new_cell_id != old_cell_id || !find_cell)
				png.plot(i,j,0.0,0.0,0.0);
			old_cell_id = new_cell_id;
		}
	}

	png.close();
}
int main(int argc, char **argv) {
	/* Check number of arguments */
	if(argc < 2) {
	  Helios::Log::error() << "Usage : " << argv[0] << " <filename>" << Helios::Log::endl;
	  exit(1);
	}

	/* Geometry */
	Geometry* geometry = new Geometry;
	/* Parser (XML for now) */
	Parser* parser = new XmlParser(*geometry);

	string filename = string(argv[1]);

	try {
		/* Read the input file */
		Log::ok() << "Reading file " + filename << Log::endl;
		parser->parseFile(filename);
	} catch(Parser::ParserError& parsererror) {
		Log::error() << "Error parsing file : " + filename + "." << Log::endl;
		/* Nothing to do, just print the message and exit */
		Log::error() << parsererror.what() << Log::endl;
		return 1;
	} catch(Parser::KeywordParserError& keyerror) {
		Log::error() << "Error parsing file : " + filename << Log::endl;
		/* Try to find the -bad- keyword */
		size_t line = seachKeyWords(filename,keyerror.getKeys());
		if(line)
			Log::error() << "Line " << (line + 1) << " : " << keyerror.what() << Log::endl;
		else
			Log::error() << keyerror.what() << Log::endl;
		return 1;
	}

	plot(*geometry,-3.6,3.6,-3.6,3.6,"test.png");

	delete geometry;
	delete parser;
}
