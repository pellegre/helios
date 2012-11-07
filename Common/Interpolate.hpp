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

#ifndef INTERPOLATE_HPP_
#define INTERPOLATE_HPP_

#include <algorithm>

namespace Helios {

	/* Interpolate a value and returns the interpolation factor (floating point) and the index (unsigned integer) */
	template<class Iterator, class T>
	std::pair<size_t, double> interpolate(Iterator begin, Iterator end, const T& value) {
		typedef typename std::iterator_traits<Iterator>::value_type ValueType;
		typedef typename std::iterator_traits<Iterator>::difference_type DistanceType;

		/* Maximum and minimum values */
		ValueType min_value = *begin;
		ValueType max_value = *(end - 1);

		/* Size of the grid */
		DistanceType size = end - begin;

		/* First check if the given energy is out of bound */
		if(value <= min_value)
			return std::pair<size_t,double>(0,0.0);
		else if(value >= max_value)
			return std::pair<size_t,double>(size - 2,1.0);

		/* Get lower index */
		DistanceType idx = std::upper_bound(begin, end, value) - begin - 1;

		/* Energy bounds */
		ValueType low = *(begin + idx);
		ValueType high = *(begin + idx + 1);

		/* Return factor */
		return std::pair<size_t,double>(idx,(value - low) / (high - low));
	}

}

#endif /* INTERPOLATE_HPP_ */
