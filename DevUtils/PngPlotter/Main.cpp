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
#include <boost/program_options.hpp>

#include "../../Parser/ParserTypes.hpp"
#include "../../Common/Common.hpp"
#include "../../Geometry/Geometry.hpp"
#include "../../Material/MaterialContainer.hpp"
#include "pngwriter.hpp"
#include "PngPlotter.hpp"

using namespace std;
using namespace Helios;
namespace po = boost::program_options;

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

int main(int argc, char **argv) {

	/* Print header, always */
	Log::header();

	/* Map of command line values */
    po::variables_map vm;

	/* Parse command line options */
	double optFloat;
	int optInt;
	/* Generic options */
	po::options_description generic("Generic options");
	generic.add_options()
		("version,v", "print version string")
		("help,h", "produce help message")
		;

	/* Declare a group of options that configure the plotter */
	po::options_description config("Configuration");
	config.add_options()
		("width,w", po::value<double>(&optFloat)->default_value(50.0),
			  "width of the plot")
		("height,h", po::value<double>(&optFloat)->default_value(50.0),
			   "height of the plot")
		("pixel,p", po::value<int>(&optInt)->default_value(1000),
			   "pixel of the plot")
		("axis,a", po::value<int>(&optInt)->default_value(2),
			    "view (2 = xy ; 1 = xz ; 0 = yz)")
		("output,o", po::value<string>(),
		        "name of the output PNG file ")
		;

	/* Hidden options (input files) */
	po::options_description hidden("Hidden options");
	hidden.add_options()
		("input-file", po::value< vector<string> >(), "input file")
		;

	po::options_description cmdline_options;
	cmdline_options.add(generic).add(config).add(hidden);

	po::options_description visible("Allowed options");
	visible.add(generic).add(config);

	po::positional_options_description p;
	p.add("input-file", -1);

	try {

        store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
        	Log::msg() << visible << endl;
            return 0;
        }

        if (vm.count("version"))
        	return 0;

    }
    catch(exception& e)
    {
        cout << e.what() << endl;
        return 1;
    }

	/* Container of filenames */
    vector<string> input_files;
    if(vm.count("input-file"))
    	input_files = vm["input-file"].as< vector<string> >();
    else {
    	Log::msg() << visible << endl;
    	return 1;
    }

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;
	try {

		for(vector<string>::const_iterator it = input_files.begin() ; it != input_files.end() ; ++it)
			parser->parseFile((*it));

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

	/* Dimensions of the graph */
	double x = vm["width"].as<double>();
	double y = vm["height"].as<double>();
	/* Pixel on the graph */
	int pixel = vm["pixel"].as<int>();
	/* View */
	int view = vm["axis"].as<int>();
	if (view < 0 || view > 2) {
    	Log::error() << "Invalid axis" << endl;
    	return 1;
	}

	/* Initialize plotter */
	PngPlotter pngPlotter(x,y,pixel);

	if(view == 0) {
		string output;
	    if(vm.count("output"))
	    	output = vm["output"].as<string>();
	    else
	    	output = "view-" + Helios::getPlaneName<0>() + ".png";
		pngPlotter.plotCell<0>(&geometry,output);
	} else if (view == 1) {
		string output;
	    if(vm.count("output"))
	    	output = vm["output"].as<string>();
	    else
	    	output = "view-" + Helios::getPlaneName<1>() + ".png";
		pngPlotter.plotCell<1>(&geometry,output);
	} else if(view == 2) {
		string output;
	    if(vm.count("output"))
	    	output = vm["output"].as<string>();
	    else
	    	output = "view-" + Helios::getPlaneName<2>() + ".png";
		pngPlotter.plotCell<2>(&geometry,output);
	}

	delete parser;
}
