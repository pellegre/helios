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
#include "../../Material/MaterialContainer.hpp"
#include "pngwriter.hpp"

using namespace std;
using namespace Helios;

static double colorFromCell(const InternalCellId& cell_id, const InternalCellId& max_id) {
	return (double)cell_id / (double)max_id;
}

static pair<string,size_t> seachKeyWords(const vector<string>& files, vector<string> search_keys) {

	/* Pair of file and match */
	pair<string,size_t> file_line;
	/* Count matches of bad keyword on the line */
	map<size_t,pair<string,size_t> > line_match;

	for(vector<string>::const_iterator it_file = files.begin() ; it_file != files.end() ; ++it_file) {
		string filename = (*it_file);
		string line;
		ifstream file (filename.c_str());
		size_t counter = 0;

		if (file.is_open()) {
			while (file.good()) {
				getline (file,line);
				bool find = true;
				size_t nfound = 0;
				for(size_t key = 0 ; key < search_keys.size() ; key++) {
					bool found = line.find(search_keys[key]) != string::npos;
					find &= found;
					if(found)
						/* Count a match for this line */
						nfound++;
				}
				if(find) return pair<string,size_t>(filename,counter);
				else line_match[nfound] = pair<string,size_t>(filename,counter);

				counter++;
			}
			file.close();
		}
	}

	/* Return the better match */
	return (*(--line_match.end())).second;
}

void plot(const Helios::Geometry& geo, const Helios::MaterialContainer& materials,
		  double xmin, double xmax, double ymin, double ymax, const std::string& filename) {
	static int pixel = 2500;
	pngwriter png(pixel,pixel,1.0,filename.c_str());
	/* Deltas */
	double deltax = (xmax - xmin) / (double)(pixel);
	double deltay = (ymax - ymin) / (double)(pixel);
	/* Number of cells */
	size_t max_id = materials.getMaterialNumber();
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
			find_cell = geo.findCell(point);
			InternalCellId new_cell_id = 0;
			if(find_cell)
				new_cell_id = find_cell->getInternalId();
			else
				continue;
			if(new_cell_id != old_cell_id) {
				png.plot(i,j,0.0,0.0,0.0);
			} else {
				const Material* cell_mat = find_cell->getMaterial();
				if(cell_mat) {
					InternalMaterialId matid = cell_mat->getInternalId();
					double color = colorFromCell(matid,max_id);
					png.plotHSV(i,j,color,1.0,1.0);
				}
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
			else
				continue;
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

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;

	/* Container of filenames */
	vector<string> input_files;

	try {

		size_t arg_count = 0;
		while(*(++argv)) {
			arg_count++;
			string filename = string(*(argv));
			input_files.push_back(filename);
			parser->parseFile(filename);
			if(arg_count == argc - 3) break;
		}

	} catch(Parser::ParserError& parsererror) {

		/* Nothing to do, just print the message and exit */
		Log::error() << parsererror.what() << Log::endl;
		return 1;

	} catch(Parser::KeywordParserError& keyerror) {

		pair<string,size_t> file_line = seachKeyWords(input_files,keyerror.getKeys());

		if(file_line.second) {
			Log::error() << "Error parsing file : " + file_line.first << Log::endl;
			Log::error() << "Line " << (file_line.second + 1) << " : " << keyerror.what() << Log::endl;
		}
		else {
			Log::error() << keyerror.what() << Log::endl;
		}

		return 1;
	}

	/* Setup problem */
	std::vector<GeometricDefinition*> geometryDefinitions = parser->getGeometry();
	std::vector<Material::Definition*> materialDefinitions = parser->getMaterials();

	/* Geometry */
	Geometry geometry(geometryDefinitions);
	/* Materials */
	MaterialContainer materials(materialDefinitions);
	/* Connect cell with materials */
	geometry.setupMaterials(materials);

	Log::ok() << "Plotting..." << Log::endl;

	double x = fromString<double>(string(*(++argv)));
	double y = fromString<double>(string(*(++argv)));
	plot(geometry,materials,-x,x,-y,y,"test.png");

	delete parser;
}
