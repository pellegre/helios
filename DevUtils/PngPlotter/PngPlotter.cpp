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
#include <iomanip>
#include "PngPlotter.hpp"

using namespace std;
using namespace Helios;

PngPlotter::PngPlotter(const double& width, const double& height, const int& pixel, const double& value) :
	width(width), height(height), pixel(pixel), colorMatrix(pixel,pixel), value(value) {

	Log::bok() << "Initializing PNG Plotter " << Log::endl;
	Log::msg() << Log::ident(1) << " - Size   = " << width << " x " << height << Log::endl;
	Log::msg() << Log::ident(1) << " - Pixels = " << pixel << " x " << pixel  << Log::endl ;
}

void PngPlotter::dump(const std::string& filename, const int& maxId, PixelColor pixelColor) {
	/* Now go backwards to set the black lines on the other side */
	int matId = 0, oldId = 0;
	for(int j = 0 ; j < pixel ; ++j)
		for(int i = 0 ; i < pixel ; ++i) {
			matId = colorMatrix(i,j);
			if(matId != oldId && matId >= 0) colorMatrix(i,j) = -1;
			oldId = matId;
		}

	Helios::Log::msg() << "Dumping file " << Helios::Log::BOLDWHITE << filename << Helios::Log::endl;

	/* Write the geometry on a PNG file */
	pngwriter png(pixel,pixel,1.0,filename.c_str());
	Color color;
	for(int i = 0 ; i < pixel ; ++i) {
		for(int j = 0 ; j < pixel ; ++j) {
			matId = colorMatrix(i,j);
			if(matId == 0) color = white();
			else if(matId == -1) color = black();
			else if(matId == -2) color = red();
			else color = (this->*pixelColor)(matId,maxId);
			png.plotHSV(i,j,color[0],color[1],color[2]);
		}
	}
	png.close();
}

void PngPlotter::dumpPng(const std::string& filename) {
	/* Get max value */
	ColorMatrix::iterator itMax = std::max_element(colorMatrix.begin(), colorMatrix.end());
	/* Dump to file */
	dump(filename,*itMax + 2,&PngPlotter::colorFromCell);
}

void PngPlotter::dumpPngNoColor(const std::string& filename) {
	/* Get max value */
	ColorMatrix::iterator itMax = std::max_element(colorMatrix.begin(), colorMatrix.end());
	/* Dump to file */
	dump(filename,*itMax + 2,&PngPlotter::white);
}
