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

#ifndef PNGPLOTTER_HPP_
#define PNGPLOTTER_HPP_

#include <string>
#include <algorithm>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include "../../Parser/ParserTypes.hpp"
#include "../../Log/Log.hpp"
#include "../../Geometry/Geometry.hpp"
#include "../../Material/MaterialContainer.hpp"
#include "pngwriter.hpp"

namespace Helios {

class PngPlotter {
	/* Color Matrix */
	typedef TinyVector<double,3> Color;
	typedef Array<int,2> ColorMatrix;

	/* Plotting parameters */
	double width;
	double height;
	int pixel;
	ColorMatrix colorMatrix;

	/* Return a color is HSV format (tune the saturation value) */
	Color colorFromCell(const int& cellId, const int& maxId) const {
		return Color((double)cellId / (double)maxId , 1.0, 1.0);
	}
	Color white() const {return Color(0.0,0.0,1.0);}
	Color black() const {return Color(0.0,0.0,0.0);}

	/* Find materials in cells */
	template<int axis>
	class MaterialFinder {

		/* General information */
		const int pixel;
		const Geometry* geometry;
		ColorMatrix& colorMatrix;

		/* Step to look for cells */
		const double xmin;
		const double xmax;
		const double ymin;
		const double ymax;
		const double deltax;
		const double deltay;

		template<int uaxis>
		static Helios::Coordinate setCoordinate(const double&x, const double&y) {
			switch(uaxis) {
			case xaxis :
				return Helios::Coordinate(0.0,x,y);
				break;
			case yaxis :
				return Helios::Coordinate(y,0.0,x);
				break;
			case zaxis :
				return Helios::Coordinate(x,y,0.0);
				break;
			}
			return Helios::Coordinate();
		}

	public:
		MaterialFinder(const double& width, const double& height, const int& pixel,
				   const Geometry* geometry, ColorMatrix& colorMatrix) :
				   pixel(pixel), geometry(geometry), colorMatrix(colorMatrix),
				   xmin(-width), xmax(width), ymin(-height), ymax(height),
				   deltax((xmax - xmin) / (double)(pixel)),
				   deltay((ymax - ymin) / (double)(pixel)) {/* */}

		void operator() (const tbb::blocked_range<int>& r) const {
			int matId = 0, oldId = 0;
			for(int i = r.begin() ; i < r.end() ; ++i)
				for(int j = 0 ; j < pixel ; ++j) {
					double x = xmin + (double)i * deltax;
					double y = ymin + (double)j * deltay;
					/* Set point on the space */
					Coordinate point(setCoordinate<axis>(x,y));
					/* Find cell */
					const Cell* findCell = geometry->findCell(point);
					if(findCell) {
						/* Get material */
						const Material* material = findCell->getMaterial();
						if(material) {
							matId = material->getInternalId();
							if(matId != oldId) colorMatrix(i,j) = -1;
							else colorMatrix(i,j) = matId;
							oldId = matId;
						} else colorMatrix(i,j) = 0;
					} else colorMatrix(i,j) = 0;
				}
			/* Now go backwards to set the black lines on the other side */
			matId = 0, oldId = 0;
			for(int j = 0 ; j < pixel ; ++j)
				for(int i = r.begin() ; i < r.end() ; ++i) {
					if(matId != oldId) colorMatrix(i,j) = -1;
					oldId = matId;
				}
		}

		~MaterialFinder() {/* */}
	};

	/* Find cells */
	template<int axis>
	class CellFinder {

		/* General information */
		const int pixel;
		const Geometry* geometry;
		ColorMatrix& colorMatrix;

		/* Step to look for cells */
		const double xmin;
		const double xmax;
		const double ymin;
		const double ymax;
		const double deltax;
		const double deltay;

		template<int uaxis>
		static Helios::Coordinate setCoordinate(const double&x, const double&y) {
			switch(uaxis) {
			case xaxis :
				return Helios::Coordinate(0.0,x,y);
				break;
			case yaxis :
				return Helios::Coordinate(y,0.0,x);
				break;
			case zaxis :
				return Helios::Coordinate(x,y,0.0);
				break;
			}
			return Helios::Coordinate();
		}

	public:
		CellFinder(const double& width, const double& height, const int& pixel,
				   const Geometry* geometry, ColorMatrix& colorMatrix) :
				   pixel(pixel), geometry(geometry), colorMatrix(colorMatrix),
				   xmin(-width), xmax(width), ymin(-height), ymax(height),
				   deltax((xmax - xmin) / (double)(pixel)),
				   deltay((ymax - ymin) / (double)(pixel)) {/* */}

		void operator() (const tbb::blocked_range<int>& r) const {
			int cellId = 0, oldId = 0;
			for(int i = r.begin() ; i < r.end() ; ++i)
				for(int j = 0 ; j < pixel ; ++j) {
					double x = xmin + (double)i * deltax;
					double y = ymin + (double)j * deltay;
					/* Set point on the space */
					Coordinate point(setCoordinate<axis>(x,y));
					/* Find cell */
					const Cell* findCell = geometry->findCell(point);
					if(findCell) {
						cellId = findCell->getUserId();
						if(cellId != oldId) colorMatrix(i,j) = -1;
						else colorMatrix(i,j) = cellId;
						oldId = cellId;
					} else colorMatrix(i,j) = 0;
				}
		}

		~CellFinder() {/* */}
	};

	/* Dump to PNG file */
	void dump(const std::string& filename, const int& maxId) {
		/* Now go backwards to set the black lines on the other side */
		int matId = 0, oldId = 0;
		for(int j = 0 ; j < pixel ; ++j)
			for(int i = 0 ; i < pixel ; ++i) {
				matId = colorMatrix(i,j);
				if(matId != oldId) colorMatrix(i,j) = -1;
				oldId = matId;
			}

		/* Write the geometry on a PNG file */
		pngwriter png(pixel,pixel,1.0,filename.c_str());
		Color color;
		for(int i = 0 ; i < pixel ; ++i) {
			for(int j = 0 ; j < pixel ; ++j) {
				matId = colorMatrix(i,j);
				if(matId == 0) color = white();
				else if(matId == -1) color = black();
				else color = colorFromCell(matId,maxId);
				png.plotHSV(i,j,color[0],color[1],color[2]);
			}
		}
		png.close();
	}

public:
	PngPlotter(const double& width, const double& height, const int& pixel) :
		width(width), height(height), pixel(pixel), colorMatrix(pixel,pixel) {/* */};

	double getHeight() const {
		return height;
	}

	void setHeight(double height) {
		this->height = height;
	}

	int getPixel() const {
		return pixel;
	}

	void setPixel(int pixel) {
		this->pixel = pixel;
	}

	double getWidth() const {
		return width;
	}

	void setWidth(double width) {
		this->width = width;
	}

	/* Function to plot a geometry */
	template<int axis>
	void plotMaterial(const std::string& filename,const Helios::Geometry* geometry);
	template<int axis>
	void plotCell(const std::string& filename,const Helios::Geometry* geometry);

	virtual ~PngPlotter(){/* */};

};

template<int axis>
void PngPlotter::plotMaterial(const std::string& filename,const Helios::Geometry* geometry) {
	/* Get the color matrix */
	tbb::parallel_for(tbb::blocked_range<int>(0,pixel),MaterialFinder<axis>(width,height,pixel,geometry,colorMatrix));
	/* Get max value */
	ColorMatrix::iterator itMax = std::max_element(colorMatrix.begin(), colorMatrix.end());
	/* Dump to file */
	dump(filename,*itMax + 2);
}

template<int axis>
void PngPlotter::plotCell(const std::string& filename,const Helios::Geometry* geometry) {
	/* Get the color matrix */
	tbb::parallel_for(tbb::blocked_range<int>(0,pixel),CellFinder<axis>(width,height,pixel,geometry,colorMatrix));
	/* Get max value */
	ColorMatrix::iterator itMax = std::max_element(colorMatrix.begin(), colorMatrix.end());
	/* Dump to file */
	dump(filename,*itMax + 2);
}
} /* namespace Helios */
#endif /* PNGPLOTTER_HPP_ */
