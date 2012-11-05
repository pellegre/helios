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

#ifndef ELASTICSCATTERING_HPP_
#define ELASTICSCATTERING_HPP_

#include "../../Isotope.hpp"

namespace Helios {

namespace AceReaction {

	/* Elastic scattering using free gas treatment */
	class ElasticScattering : public Helios::Reaction {
		/* Atomic weight ratio */
		double awr;
		/* Temperature (in MeVs) */
		double temperature;
		/* Sample target velocity. Adapted from SERPENT */
		void targetVelocity(double energy, Direction direction, Direction& velocity, Random& random);
	public:
		/* Threshold values */
		static double energy_freegas_threshold;
		static double awr_freegas_threshold;

		ElasticScattering(double awr, double temperature) : awr(awr), temperature(temperature) {/* */};

		/* Change particle state */
		void operator()(Particle& particle, Random& r) const;

		virtual ~ElasticScattering() {/* */};
	};

}

} /* namespace Helios */
#endif /* ELASTICSCATTERING_HPP_ */
