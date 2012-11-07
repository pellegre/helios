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
 DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NEUTRONREACTION_HPP_
#define NEUTRONREACTION_HPP_

#include <vector>
#include <fstream>
#include "CrossSection.hpp"
#include "AngularDistribution.hpp"
#include "EnergyDistribution.hpp"
#include "TyrDistribution.hpp"

namespace Ace {

	class NeutronReaction {

		/* Number of the ENDF number for the neutron reaction */
		int mt;

		/* Kinematic Q-value for the neutron reaction */
		double q;

		/*
		 * Information about the reaction (includes the number of secondary
		 * neutrons and whether secondary neutron angular distributions are
		 * in the laboratory or center-of-mass system.
		 */

		/* Cross section of the reaction */
		CrossSection xs;

		/* Angular Distribution of the reaction */
		AngularDistribution adist;

		/* Energy distribution of the reaction */
		EnergyDistribution edist;

		/* In case there is a distribution in energy of neutron released */
		TyrDistribution tyr_dist;

	public:

		NeutronReaction(int mt, double q, const TyrDistribution& tyr_dist, const CrossSection& xs, const AngularDistribution& adist,const EnergyDistribution& edist) :
			mt(mt), q(q), xs(xs), adist(adist), edist(edist), tyr_dist(tyr_dist) {/* */};

		/* Get Reaction values */
		int getMt() const {return mt;};
		double getQ() const {return q;};

		const TyrDistribution& getTyr() const {return tyr_dist;};
		TyrDistribution& getTyr() {return tyr_dist;};

		const CrossSection& getXs() const {return xs;};
		CrossSection& getXs() {return xs;};

		const AngularDistribution& getAngular() const {return adist;};
		AngularDistribution& getAngular() {return adist;};

		const EnergyDistribution& getEnergy() const {return edist;};
		EnergyDistribution& getEnergy() {return edist;};

		void printInformation(std::ostream& out) const;

		virtual ~NeutronReaction() { /* */ };
	};

} /* namespace ACE */

#endif /* NEUTRONREACTION_HPP_ */
