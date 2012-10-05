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

#include "Parser/ParserTypes.hpp"
#include "Log/Log.hpp"
#include "Geometry/Geometry.hpp"
#include "Material/MaterialContainer.hpp"
#include "Transport/Particle.hpp"

using namespace std;
using namespace Helios;

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
	/* Check number of arguments */
	if(argc < 2) {
	  Helios::Log::error() << "Usage : " << argv[0] << " <filename>" << Helios::Log::endl;
	  exit(1);
	}

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;
	/* Geometry */
	Geometry* geometry;
	/* Materials */
	MaterialContainer* materials;

	/* Container of filenames */
	vector<string> input_files;

	try {

		while(*(++argv)) {
			string filename = string(*(argv));
			input_files.push_back(filename);
			parser->parseFile(filename);
		}

		/* Setup problem */
		geometry = parser->getGeometry();
		materials = parser->getMaterials();

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

	/* Connect cell with materials */
	map<MaterialId, InternalMaterialId> materialMap = materials->getMaterialMap();
	vector<Material*> materialPtrs = materials->getMaterials();
	vector<Cell*> cells = geometry->getCells();
	for(vector<Cell*>::const_iterator it = cells.begin() ; it != cells.end() ; ++it) {
		MaterialId cellMatId = (*it)->getMaterialId();
		if(cellMatId != Material::NONE) {
			if(!(*it)->getFill()) {
				map<MaterialId,InternalMaterialId>::const_iterator it_mat = materialMap.find(cellMatId);
				if(it_mat == materialMap.end()) {
					Log::error() << "Material *" + cellMatId + "* is not defined" << Log::endl;
					exit(1);
				}
				else {
					(*it)->setMaterial(materialPtrs[(*it_mat).second]);
				}
			}
			else {
				Log::error() << "Material " + cellMatId + " is not filled with a material or universe" << Log::endl;
				exit(1);
			}
		}
	}

	/* Initialization - Particle */
	trng::lcg64 r;
	r.seed((long unsigned int)time(0));
	Particle particle(Coordinate(0,0,0),Direction(-1.0,0,0),EnergyPair(),1.0,r);
	/* Random initial direction */
	isotropicDirection(particle);

	/* Initialization - Geometry stuff */
	const Cell* cell(geometry->findCell(particle.pos()));
	Surface* surface(0);
	bool sense(true);
	double distance(0.0);

	bool out = false;

	while(true) {

	    /* Russian roulette */
	    if(particle.wgt() < 0.001) {
	    	if (particle.random() < 0.25) break;
	    	else particle.wgt() /= 0.25;
	    }

		/* Get next surface and distance */
		cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

		/* Energy index of the particle */
		EnergyIndex energy_index = particle.eix();
		/* Get material */
		const Material* material = cell->getMaterial();
		/* Get total cross section */
		double total_xs = material->getTotalXs(energy_index);

		/* Get collision distance */
		double collision_distance = -log(particle.random())/total_xs;

		while(collision_distance > distance) {

			/* Cut on a vacuum surface */
			if(surface->getFlags() & Surface::VACUUM) {
				out = true;
				break;
			}

			/* Transport the particle to the surface */
			particle.pos() = particle.pos() + distance * particle.dir();

			/* Now get next cell */
			surface->cross(particle.pos(),sense,cell);

			/* Cut if the cell is dead */
			if(cell->getFlag() & Cell::DEADCELL) {
				out = true;
				break;
			}

			/* Calculate new distance to the closest surface */
			cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

			/* Update material */
			material = cell->getMaterial();
			total_xs = material->getTotalXs(energy_index);

			/* And the new collision distance */
			collision_distance = -log(particle.random())/total_xs;
		}

		if(out) break;

		/* If we are out, we reach some point inside a cell were a collision should occur */
		particle.pos() = particle.pos() + collision_distance * particle.dir();

		/* Modify particle weight */
		double abs_xs = material->getAbsorptionXs(energy_index);
		particle.wgt() *= 1.0 - (abs_xs / total_xs);

		/* The particle collide with the material */
		material->collision(particle);

		cout << particle << endl;

	}

	delete materials;
	delete geometry;
	delete parser;
}
