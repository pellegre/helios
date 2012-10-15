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

#include "../../Common/Common.hpp"
#include "../../Parser/ParserTypes.hpp"
#include "../../Geometry/Geometry.hpp"
#include "../../Material/MaterialContainer.hpp"
#include "pngwriter.hpp"

class PngPlotter {

	/* Color Matrix */
	typedef Helios::Coordinate Color;
	typedef Helios::IntMatrix ColorMatrix;

	/* Plotting parameters */
	double width;
	double height;
	int pixel;
	ColorMatrix colorMatrix;

	/* Return a color is HSV format (tune the saturation value) */
	Color colorFromCell(const int& cellId, const int& maxId) const {
		return Color((double)cellId / (double)maxId , 1.0, 1.0);
	}
	Color white(const int& cellId = 0, const int& maxId = 0) const {return Color(0.0,0.0,1.0);}
	Color black(const int& cellId = 0, const int& maxId = 0) const {return Color(0.0,0.0,0.0);}
	Color red(const int& cellId = 0, const int& maxId = 0) const {return Color(1.0,1.0,1.0);}

	/* Function to select a color for the pixel */
	typedef Color (PngPlotter::*PixelColor)(const int&, const int&) const;

	/* Find materials in cells */
	template<int axis>
	class MaterialFinder {

		/* General information */
		const int pixel;
		const Helios::Geometry* geometry;
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
			case Helios::xaxis :
				return Helios::Coordinate(0.0,x,y);
				break;
			case Helios::yaxis :
				return Helios::Coordinate(y,0.0,x);
				break;
			case Helios::zaxis :
				return Helios::Coordinate(x,y,0.0);
				break;
			}
			return Helios::Coordinate();
		}

	public:
		MaterialFinder(const double& width, const double& height, const int& pixel,
				   const Helios::Geometry* geometry, ColorMatrix& colorMatrix) :
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
					Helios::Coordinate point(setCoordinate<axis>(x,y));
					/* Find cell */
					const Helios::Cell* findCell = geometry->findCell(point);
					if(findCell) {
						/* Get material */
						const Helios::Material* material = findCell->getMaterial();
						if(material) {
							matId = material->getInternalId() + 1;
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
		const Helios::Geometry* geometry;
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
			case Helios::xaxis :
				return Helios::Coordinate(0.0,x,y);
				break;
			case Helios::yaxis :
				return Helios::Coordinate(y,0.0,x);
				break;
			case Helios::zaxis :
				return Helios::Coordinate(x,y,0.0);
				break;
			}
			return Helios::Coordinate();
		}

	public:
		CellFinder(const double& width, const double& height, const int& pixel,
				   const Helios::Geometry* geometry, ColorMatrix& colorMatrix) :
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
					Helios::Coordinate point(setCoordinate<axis>(x,y));
					/* Find cell */
					const Helios::Cell* findCell = geometry->findCell(point);
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

	/* Find materials in cells */
	template<int axis>
	class SourceSimulator {

		/* General information */
		const int pixel;
		const Helios::Source* source;
		ColorMatrix& colorMatrix;
		Helios::Random r;

		/* Step to look for cells */
		const double xmin;
		const double xmax;
		const double ymin;
		const double ymax;
		const double deltax;
		const double deltay;

	public:
		SourceSimulator(const double& width, const double& height, const int& pixel,
				   const Helios::Source* source, const Helios::Random &r,  ColorMatrix& colorMatrix) :
				   pixel(pixel), source(source), r(r), colorMatrix(colorMatrix),
				   xmin(-width), xmax(width), ymin(-height), ymax(height),
				   deltax((xmax - xmin) / (double)(pixel)),
				   deltay((ymax - ymin) / (double)(pixel)) {/* */}

		void operator() (const tbb::blocked_range<int>& range) const {
			/* Jump random number generator */
			Helios::Random r_local(r);
			r_local.getEngine().jump(100*range.begin());
			/* Sample particles and set the color matrix */
			for(int i = range.begin() ; i < range.end() ; ++i) {
				Helios::Particle sampleParticle = source->sample(r_local);
				/* Get coordinates */
				Helios::Coordinate position = sampleParticle.pos();
				/* Get coordinates */
				double x = Helios::getAbscissa<axis>(position);
				double y = Helios::getOrdinate<axis>(position);
				/* Get pair of values on the color matrix */
				int i = (x - xmin) / deltax;
				int j = (y - ymin) / deltay;
				/* Do some sanity check and set the pixel */
				if ((i >= 0 && i < pixel) && (j >= 0 && j < pixel))
					colorMatrix(i,j) = -2;
				else
					Helios::Log::warn() << "Source particle out of bounds" << Helios::Log::endl;
			}
		}

		~SourceSimulator() {/* */}
	};

	/* Dump to PNG file */
	void dump(const std::string& filename, const int& maxId, PixelColor pixelColor);

public:
	PngPlotter(const double& width, const double& height, const int& pixel);

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
	void plotMaterial(const Helios::Geometry* geometry);
	template<int axis>
	void plotCell(const Helios::Geometry* geometry);
	template<int axis>
	void plotSource(const Helios::Geometry* geometry, const Helios::Source* source, const Helios::Random& r, const int& npart = 1000);

	/* Dump PNG file */
	void dumpPng(const std::string& filename);
	void dumpPngNoColor(const std::string& filename);
	virtual ~PngPlotter(){/* */};

};

template<int axis>
void PngPlotter::plotMaterial(const Helios::Geometry* geometry) {
	/* Get the color matrix */
	tbb::parallel_for(tbb::blocked_range<int>(0,pixel),MaterialFinder<axis>(width,height,pixel,geometry,colorMatrix));
}

template<int axis>
void PngPlotter::plotCell(const Helios::Geometry* geometry) {
	/* Get the color matrix */
	tbb::parallel_for(tbb::blocked_range<int>(0,pixel),CellFinder<axis>(width,height,pixel,geometry,colorMatrix));
}

template<int axis>
void PngPlotter::plotSource(const Helios::Geometry* geometry, const Helios::Source* source, const Helios::Random& r, const int& npart) {
	/* Get the color matrix */
	tbb::parallel_for(tbb::blocked_range<int>(0,npart),SourceSimulator<axis>(width,height,pixel,source,r,colorMatrix));
}

#endif /* PNGPLOTTER_HPP_ */
