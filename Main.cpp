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
#include "Geometry/Cell.hpp"
#include "Material/Materials.hpp"
#include "Material/MacroXs.hpp"
#include "Transport/Particle.hpp"
#include "Transport/Source.hpp"
#include "Common/Common.hpp"
#include "Environment/McEnvironment.hpp"

using namespace std;
using namespace Helios;

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

	while(*(++argv)) {
		string filename = string(*(argv));
		input_files.push_back(filename);
	}

	/* Environment */
	McEnvironment environment(parser);
	/* Parse files, to get the information to create the environment */
	environment.parseFiles(input_files);
	/* Setup the problem */
	environment.setup();

	/* Geometry */
	Geometry* geometry = environment.getModule<Geometry>();
	geometry->printGeo(cout);

	vector<Surface*> surfaces = environment.getObject<Geometry,Surface>("1<2[0,1,0]<1");
	cout << *surfaces[0] << endl;

	/* Get materials */
	Materials* materials = environment.getModule<Materials>();
	/* Get the source */
	Source* source = environment.getModule<Source>();

	/* Initialization - Random number */
	Random r;
	r.getEngine().seed((long unsigned int)1);

	/* Initialization - KEFF cycle */
	double keff = 1.186;
	int neutrons = 25;
	int skip = 5;
	int cycles = 10;
	list<pair<const Cell*,Particle> > particles;
	/* Particle bank, the particles for the next cycle are banked here */
	list<pair<const Cell*,Particle> > fission_bank;

	for(size_t i = 0 ; i < neutrons ; ++i) {
		/* Sample particle */
        Particle p = source->sample(r);
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

	delete parser;
}
