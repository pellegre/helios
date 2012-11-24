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

#ifndef ENDFINTERPOLATE_HPP_
#define ENDFINTERPOLATE_HPP_

#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace Helios {

	/* Class to interpolate values using ENDF laws */
	class EndfInterpolate {
		/* Interpolation ranges separators */
		std::vector<int> nbt;
		/* Interpolation type on each region */
		std::vector<int> aint;
	public:
		EndfInterpolate(const std::vector<int>& nbt, const std::vector<int>& aint) : nbt(nbt), aint(aint) {
			/* Sanity check */
			assert(nbt.size() == aint.size());
		}

		/* Return interpolated value */
		template<class Iterator, class T>
		double interpolate(Iterator xbegin, Iterator xend, Iterator ybegin, Iterator yend, const T& value) const {
			typedef typename std::iterator_traits<Iterator>::value_type ValueType;
			typedef typename std::iterator_traits<Iterator>::difference_type DistanceType;

			/* Sanity check */
			assert((yend - ybegin) == (xend - xbegin));

			/* Maximum and minimum values */
			ValueType min_value = *xbegin;
			ValueType max_value = *(xend - 1);

			/* First check if the given energy is out of bound */
			if(value <= min_value) return *ybegin;
			else if(value >= max_value) return *(yend - 1);

			/* Check size of ENDF parameters */
			if(nbt.size() == 0) {
				/* Linear - Linear assumed */
				DistanceType idx = std::upper_bound(xbegin, xend, value) - xbegin - 1;
				/* Get bounds values */
				double y1 = *(ybegin + idx + 1) , y0 = *(ybegin + idx);
				double x1 = *(xbegin + idx + 1) , x0 = *(xbegin + idx);
				/* Linear-linear */
				if (x1 != x0)
					return  (y1 - y0)*(value - x0)/(x1 - x0) + y0;
				else if ((y1 == y0) || (value == x0))
					return y0;
			}
			else {
				/* Find interpolation region */
				size_t lower = 0;
				for(size_t i = 0 ; i < nbt.size() ; ++i) {
					/* High bound */
					size_t high = nbt[i];
					/* Check if we are inside this range */
					if(value > *(xbegin + lower) && value <= *(xbegin + high - 1)) {
						/* Binary search (get lower index) */
						DistanceType idx = std::upper_bound(xbegin + lower, xbegin + high, value) - xbegin - 1;
						/* Get interpolation type */
						int type = aint[i];

						/* Get bounds values */
						double y1 = *(ybegin + idx + 1) , y0 = *(ybegin + idx);
						double x1 = *(xbegin + idx + 1) , x0 = *(xbegin + idx);
						/* Return value */
						double y(y0);

						/* Get type */
						switch (type) {
						case 1:
							/* Histogram */
							y = y0;
							break;
						case 0:
						case 2:
							/* Linear-linear */
							if (x1 != x0)
								y = (y1 - y0)*(value - x0)/(x1 - x0) + y0;
							else if ((y1 == y0) || (value == x0))
								y = y0;
							break;
						case 3:
							/* Linear-log */
							y = (y1 - y0)*log(value/x0)/log(x1/x0) + y0;
							break;
						case 4:
							/* Log-linear */
							if ((y0 != 0.0) && (x1 != x0))
								y = pow(y1/y0, (value - x0)/(x1 - x0))*y0;
							break;
						case 5:
							/* Log-log */
							if (y0 != 0.0)
								y = pow(y1/y0, log(value/x0)/log(x1/x0))*y0;
							break;
						default:
							/* Default behavior */
							y = y0;
							break;
						}

						/* Return interpolated value */
						return y;
					}
					/* Update lower bound */
					lower = high - 1;
				}
			}

			return 0.0;
		}

		~EndfInterpolate() {/* */}
	};

}

#endif /* ENDFINTERPOLATE_HPP_ */
