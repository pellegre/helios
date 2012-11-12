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

#include "../AceModule.hpp"

namespace Helios {

namespace AceReaction {

	/*
	 * Elastic scattering using free gas treatment
	 *
	 * The scattering cosine is always sampled on the CM system
	 */
	template<class MuSampling>
	class ElasticScattering : public Reaction, public MuSampling {
		/* Atomic weight ratio */
		double awr;
		/* Temperature (in MeVs) */
		double temperature;
		/* Sample target velocity. */
		void targetVelocity(double energy, Direction direction, Direction& velocity, Random& random) const;
	public:
		ElasticScattering(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction) :
			MuSampling(ace_reaction.getAngular()), awr(isotope->getAwr()), temperature(isotope->getTemperature()) {/* */};

		/* Change particle state */
		void operator()(Particle& particle, Random& random) const;

		void print(std::ostream& out) const;

		virtual ~ElasticScattering() {/* */};
	};

	template<class MuSampling>
	void ElasticScattering<MuSampling>::targetVelocity(double energy, Direction direction, Direction& velocity, Random& random) const {
		/* Compare to threshold criteria */
		if ((energy > AceIsotope::energy_freegas_threshold*temperature) && (awr > AceIsotope::awr_freegas_threshold)) {
			/* Target velocity insignificant, set components to zero */
			velocity = Direction(0.0,0.0,0.0);
			return;
		}

		/*
		 * Rejection algorithm extracted from MCNP5 manual. Samples target
		 * energy z2/ar, cosine between target and neutron velocity c.
		 */
		double ar = awr/temperature;
		double ycn = sqrt(energy*ar);

		/* Auxiliary variables */
		double r1, z2, rnd1, rnd2, s, x2, c;

		/* Rejection sampling */
		do {
			if (random.uniform()*(ycn + 1.12837917) > ycn) {
				r1 = random.uniform();
				z2 = -log(r1*random.uniform());
			}
			else {
				do {
					rnd1 = random.uniform();
					rnd2 = random.uniform();

					r1 = rnd1*rnd1;
					s = r1 + rnd2*rnd2;
				}
				while (s > 1.0);

				z2 = -r1*log(s)/s - log(random.uniform());
			}

			double z = sqrt(z2);
			c = 2.0*random.uniform() - 1.0;
			x2 = ycn*ycn + z2 - 2*ycn*z*c;
			rnd1 = random.uniform()*(ycn + z);
		}
		while (rnd1*rnd1 > x2);

		/* Rotate direction cosines */
		azimutalRotation(c, direction, random);

		/* Calculate velocity components */
		double vel = sqrt(z2/ar);
		velocity = vel * direction;
	}

	template<class MuSampling>
	void ElasticScattering<MuSampling>::operator()(Particle& particle, Random& random) const {
		/* Get initial energy on LAB system */
		double particle_energy = particle.erg().second;

		/* Initial particle velocity */
		double velp = sqrt(particle_energy);

		/* Get initial velocity of the particle */
		Direction vp = velp * particle.dir();

		/* Sample initial target velocity */
		Direction vt;
		targetVelocity(particle_energy, particle.dir(), vt, random);

		/* Calculate velocity of CM */
		Direction vc = (vp + awr * vt) / (awr + 1.0);

		/* Particle velocities in CM */
		vp = vp - vc;
		velp = sqrt(dot(vp,vp));

		/* Sample scattering cosine in CM */
		double muc;
		MuSampling::setCosine(particle, random, muc);

		/* Calculate direction cosines and rotate */
		particle.dir() = vp / velp;
		azimutalRotation(muc, particle.dir(), random);

		/* Velocities after collision. */
		vp = particle.dir() * velp;

		/* Particle velocities in LAB */
		vp = vp + vc;
		velp = sqrt(dot(vp,vp));

		/* Particle energy */
		particle.erg().second = dot(vp,vp);

		/* Set direction cosines */
		particle.dir() = vp / velp;
	}

	template<class MuSampling>
	void ElasticScattering<MuSampling>::print(std::ostream& out) const {
		out << " Elastic ACE reaction" << endl;
		out << "  - awr = " << awr << endl;
		out << "  - tmp = " << temperature << endl;
	};

}

} /* namespace Helios */
#endif /* ELASTICSCATTERING_HPP_ */
