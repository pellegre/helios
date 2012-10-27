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
#include "../../Geometry/Geometry.hpp"
#include "../../Geometry/Cell.hpp"
#include "../../Material/Materials.hpp"
#include "../../Material/MacroXs.hpp"
#include "../../Transport/Particle.hpp"
#include "../../Transport/Source.hpp"
#include "../../Common/Common.hpp"
#include "../../Environment/McEnvironment.hpp"
#include "pngwriter.hpp"
#include "PngPlotter.hpp"

using namespace std;
using namespace Helios;
namespace po = boost::program_options;

/* Type of plotting */
static const int CELL = 1;
static const int MATERIAL = 2;

template<int axis, int type>
static void plotPng(PngPlotter& pngPlotter, const po::variables_map& vm, const Geometry* geometry) {
	string output;
    if(vm.count("output"))
    	output = vm["output"].as<string>();
    else
    	output = "view-" + Helios::getPlaneName<axis>() + ".png";
    if(type == CELL)
    	pngPlotter.plotCell<axis>(geometry);
    else if(type == MATERIAL)
    	pngPlotter.plotMaterial<axis>(geometry);
}

template<int axis>
static string outputFile(const po::variables_map& vm) {
	string output;
    if(vm.count("output"))
    	output = vm["output"].as<string>();
    else
    	output = "view-" + Helios::getPlaneName<axis>() + ".png";
    return output;
}

int main(int argc, char **argv) {

	/* Print header, always */
	cout << endl;
	Log::header();

	/* Map of command line values */
    po::variables_map vm;

	/* Parse command line options */
	double optFloat;
	int optInt;
	/* Generic options */
	po::options_description generic("Generic options");
	generic.add_options()
		;

	/* Declare a group of options that configure the plotter */
	po::options_description config("Configuration");
	config.add_options()
		("width,w", po::value<double>(&optFloat)->default_value(50.0),
			  "width of the plot")
		("height,h", po::value<double>(&optFloat)->default_value(50.0),
			   "height of the plot")
		("value,v", po::value<double>(&optFloat)->default_value(0.0),
			   "axis value ")
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

        if (vm.size() == 0) {
        	Log::msg() << visible << endl;
            return 0;
        }

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

	/* Environment */
	McEnvironment environment(parser);
	/* Parse files, to get the information to create the environment */
	environment.parseFiles(input_files);
	/* Setup the problem */
	environment.setup();

	/* Geometry */
	Geometry* geometry = environment.getModule<Geometry>();

	/* Get materials */
	Materials* materials = 0;
	try {
		materials = environment.getModule<Materials>();
	} catch (exception& error) {
		Log::warn() << error.what() << Log::endl;
	}

	/* Get the source */
	Source* source = 0;
	try {
		source = environment.getModule<Source>();
	} catch (exception& error) {
		Log::warn() << error.what() << Log::endl;
	}

	/* Dimensions of the graph */
	double x = vm["width"].as<double>();
	double y = vm["height"].as<double>();
	/* Value where the plane intersect the axis */
	double value = vm["value"].as<double>();
	/* Pixel on the graph */
	int pixel = vm["pixel"].as<int>();
	/* View */
	int view = vm["axis"].as<int>();
	if (view < 0 || view > 2) {
    	Log::error() << "Invalid axis" << endl;
    	return 1;
	}

	/* Output file name */
	string output = "";

	/* Initialize plotter */
	PngPlotter pngPlotter(x,y,pixel,value);

	/* Initialization - Random number */
	trng::lcg64 random;
	Random r(random);
	r.seed(1);
	/* Set the source of the problem */
	int nparticles = 100000;

	/* Dump a PNG file */
	if(view == 0) {
		if(materials)
			plotPng<0,MATERIAL>(pngPlotter,vm,geometry);
		else
			plotPng<0,CELL>(pngPlotter,vm,geometry);
		if(source)
			pngPlotter.plotSource<0>(geometry,source,r,nparticles);
		output = outputFile<0>(vm);
	} else if (view == 1) {
		if(materials)
			plotPng<1,MATERIAL>(pngPlotter,vm,geometry);
		else
			plotPng<1,CELL>(pngPlotter,vm,geometry);
		if(source)
			pngPlotter.plotSource<1>(geometry,source,r,nparticles);
		output = outputFile<1>(vm);
	} else if(view == 2) {
		if(materials)
			plotPng<2,MATERIAL>(pngPlotter,vm,geometry);
		else
			plotPng<2,CELL>(pngPlotter,vm,geometry);
		if(source)
			pngPlotter.plotSource<2>(geometry,source,r,nparticles);
		output = outputFile<2>(vm);
	}

	/* Dump the plot */
    pngPlotter.dumpPng(output);

	delete parser;
}
