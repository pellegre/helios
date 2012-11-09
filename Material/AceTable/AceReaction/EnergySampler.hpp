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

#ifndef ENERGYSAMPLER_HPP_
#define ENERGYSAMPLER_HPP_

#include "../../../Common/Common.hpp"
#include "../../../Transport/Particle.hpp"

namespace Helios {

namespace AceReaction {
	/*
	 * Base class to deal with cosine samplers
	 */
	class EnergySampler {
	public:
		EnergySampler(const Ace::EnergyDistribution& ace_data) {/* */}

		/* Exception */
		class BadEnergySamplerCreation : public std::exception {
			std::string reason;
		public:
			BadEnergySamplerCreation(const std::string& msg) {
				reason = "Cannot create energy sampler : " + msg;
			}
			const char *what() const throw() {
				return reason.c_str();
			}
			~BadEnergySamplerCreation() throw() {/* */};
		};

		/* Sample energy (and MU if information exists) using particle's information */
		virtual void setEnergy(const Particle& particle, Random& random, double& energy, double& mu) const = 0;

		/* Print internal data of the energy sampler */
		virtual void print() const = 0;

		virtual ~EnergySampler() {/* */}
	};


	class EnergySamplerFactory {
	public:
		EnergySamplerFactory() {/* */}
		EnergySampler* createSampler(const Ace::EnergyDistribution& ace_data) {
			/* Get law */
			int law = ace_data.laws[0]->getLaw();
			/* No law */
			throw(EnergySampler::BadEnergySamplerCreation("Energy law " + toString(law) + " is not supported"));
		}
		~EnergySamplerFactory() {/* */}
	};
}

}

#endif /* ENERGYSAMPLER_HPP_ */
