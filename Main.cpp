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
#include <list>

#include "Parser/ParserTypes.hpp"
#include "Geometry/Geometry.hpp"
#include "Material/MaterialContainer.hpp"
#include "Transport/Particle.hpp"
#include "Transport/Distribution.hpp"
#include "Transport/Source.hpp"
#include "Common/Common.hpp"

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

	/* Print header */
	Log::header();

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;

	/* Container of filenames */
	vector<string> input_files;

	try {

		while(*(++argv)) {
			string filename = string(*(argv));
			input_files.push_back(filename);
			parser->parseFile(filename);
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
	Geometry* geometry = new Geometry(geometryDefinitions);
	/* Materials */
	MaterialContainer* materials = new MaterialContainer(materialDefinitions);
	/* Connect cell with materials */
	geometry->setupMaterials(*materials);

	/* Initialization - Random number */
	trng::lcg64 random;
	Random r(random);
	r.getEngine().seed((long unsigned int)1);

	/* Initialization - KEFF cycle */
	double keff = 1.186;
	int neutrons = 5000;
	int skip = 50;
	int cycles = 250;
	double ax = -32.13;
	double bx =  10.71;
	double ay = -10.71;
	double by =  32.13;
	list<pair<const Cell*,Particle> > particles;
	/* Fission bank, the particles for the next cycle are banked here */
	list<pair<const Cell*,Particle> > fission_bank;

	vector<double> coeffs;
	coeffs.push_back(ax);
	coeffs.push_back(bx);
	coeffs.push_back(ay);
	coeffs.push_back(by);
	DistributionBase* box
	   = DistributionFactory::access().createDistribution(new DistributionBase::Definition("box2d-xy","0",coeffs));
	DistributionBase* iso
	   = DistributionFactory::access().createDistribution(new DistributionBase::Definition("isotropic","1",vector<double>()));
	SamplerId id = 0;
	ParticleSampler::Definition defSampler(id,Coordinate(0,0,0),Direction(0,0,0),vector<DistributionId>());
	vector<DistributionBase*> distCont;
	distCont.push_back(box);
	distCont.push_back(iso);
	defSampler.setDistributions(distCont);
	ParticleSampler* partSampler = new ParticleSampler(&defSampler);

	vector<SamplerId> samplerIds(1);
	vector<double> weight(1);
	Source::Definition defSource(samplerIds,weight);
	vector<ParticleSampler*> samplers;
	samplers.push_back(partSampler);
	defSource.setSamplers(samplers);

	Source* source = new Source(&defSource);

	for(size_t i = 0 ; i < neutrons ; ++i) {
		Coordinate initial_pos(0.0,0.0,0.0);
		Direction initial_dir(0.0,0.0,0.0);

//		double x = (bx - ax)*r.uniform() + ax;
//		double y = (by - ay)*r.uniform() + ay;
//		Coordinate initial_pos(x,y,0.0);
//		isotropicDirection(initial_dir,r);

        Particle p = Particle(initial_pos,initial_dir,EnergyPair(0,0.0),1.0);
//        (*box)(p,r);
//		(*iso)(p,r);
		source->sample(p,r);
		const Cell* c(geometry->findCell(p.pos()));
		particles.push_back(pair<const Cell*,Particle>(c,p));
	}

	/* Initialization - Geometry stuff */
	Surface* surface(0);
	bool sense(true);
	double distance(0.0);
	double ave_keff = 0.0;

	for(int ncycle = 0 ; ncycle <= cycles ; ++ncycle) {

		/* Update new particles from the fission bank */
		while(!fission_bank.empty()) {
			/* Get banked particle */
			pair<const Cell*,Particle> banked_particle = fission_bank.back();
			fission_bank.pop_back();
			/* Split particle */
			double amp = banked_particle.second.wgt() / keff;
			int split = std::max(1,(int)(amp));
			/* New weight of the particle */
			banked_particle.second.wgt() = amp/(double)split;
			/* Put the split particle into the "simulation" list */
			banked_particle.second.sta() = Particle::ALIVE;
			for(int i = 0 ; i < split ; i++)
				particles.push_back(banked_particle);
		}

		/* Initialize counters */
		double pop = 0.0;

		while(!particles.empty()) {

			pair<const Cell*,Particle> pc = particles.back();
			particles.pop_back();

			const Cell* cell = pc.first;
			Particle particle = pc.second;

			bool out = false;

			while(true) {

				/* Get next surface and distance */
				cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

				/* Energy index of the particle */
				EnergyIndex energy_index = particle.eix();
				/* Get material */
				const Material* material = cell->getMaterial();
				/* Get total cross section */
				double mfp = material->getMeanFreePath(energy_index);

				/* Get collision distance */
				double collision_distance = -log(r.uniform())*mfp;

				while(collision_distance > distance) {
					/* Cut on a vacuum surface */
					if(surface->getFlags() & Surface::VACUUM) {
						out = true;
						break;
					}

					/* Transport the particle to the surface */
					particle.pos() = particle.pos() + distance * particle.dir();

					if(surface->getFlags() & Surface::REFLECTING) {
						/* Get normal */
						Direction normal;
						surface->normal(particle.pos(),normal);
						/* Reverse if necessary */
						if(sense == false) normal = -normal;
						/* Calculate the new direction */
						double projection = 2 * dot(particle.dir(), normal);
						particle.dir() = particle.dir() - projection * normal;
					} else {
						/* Now get next cell */
						surface->cross(particle.pos(),sense,cell);
						if(!cell) {
							cout << particle << endl;
							cout << *surface << endl;
						}
						/* Cut if the cell is dead */
						if(cell->getFlag() & Cell::DEADCELL) {
							out = true;
							break;
						}
					}

					/* Calculate new distance to the closest surface */
					cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

					/* Update material */
					material = cell->getMaterial();
					mfp = material->getMeanFreePath(energy_index);

					/* And the new collision distance */
					collision_distance = -log(r.uniform())*mfp;
				}

				if(out) break;

				/* If we are out, we reach some point inside a cell were a collision should occur */
				particle.pos() = particle.pos() + collision_distance * particle.dir();

				/* get and apply reaction to the particle */
				Reaction* reaction = material->getReaction(particle.eix(),r);
				(*reaction)(particle,r);

				if(particle.sta() == Particle::DEAD) break;
				if(particle.sta() == Particle::BANK) {
					pop += particle.wgt();
					fission_bank.push_back(pair<const Cell*,Particle>(cell,particle));
					break;
				}
			}
		}

		/* Calculate multiplication factor */
		keff = pop / (double)neutrons;
		if(ncycle > skip) {
			Log::ok() << "Cycle = " << ncycle << " - keff = " << keff << Log::endl;
			ave_keff += keff;
		} else {
			Log::ok() << " Cycle (Inactive) = " << ncycle << " - keff = " << keff << Log::endl;
		}
	}

	Log::ok() << " Final keff = "<< ave_keff/ (double)(cycles - skip) << Log::endl;

	delete materials;
	delete geometry;
	delete parser;
}
