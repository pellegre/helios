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

#include "Utils.hpp"
#define NO_FREETYPE
#include "pngwriter.hpp"

using namespace std;
using namespace Helios;

size_t seachKeyWords(const string& filename, vector<string> search_keys) {
	string line;
	ifstream file (filename.c_str());
	size_t counter = 0;
	if (file.is_open()) {
		while (file.good()) {
			getline (file,line);
			bool find = true;
			for(size_t key = 0 ; key < search_keys.size() ; key++)
				find &= (line.find(search_keys[key]) != string::npos);
			if(find) break;
			counter++;
		}
		file.close();
	}
	return counter;
}

static double colorFromCell(const InternalCellId& cell_id, const InternalCellId& max_id) {
	return (double)cell_id / (double)max_id;
}


const std::string trim(const std::string& pString,const std::string& pWhitespace)
{
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == std::string::npos)
    {
        // no content
        return "";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}

const std::string reduce(const std::string& pString,const std::string& pFill,const std::string& pWhitespace)
{
    // trim first
    std::string result(trim(pString, pWhitespace));

    // replace sub ranges
    size_t beginSpace = result.find_first_of(pWhitespace);
    while (beginSpace != std::string::npos)
    {
        const size_t endSpace =
                        result.find_first_not_of(pWhitespace, beginSpace);
        const size_t range = endSpace - beginSpace;

        result.replace(beginSpace, range, pFill);

        const size_t newStart = beginSpace + pFill.length();
        beginSpace = result.find_first_of(pWhitespace, newStart);
    }

    return result;
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
			/* Get cell ID */
			find_cell = geo.findCell(Coordinate(x,y,0.0));
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
