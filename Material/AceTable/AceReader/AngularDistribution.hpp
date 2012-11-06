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

#ifndef ANGULARDISTRIBUTION_HPP_
#define ANGULARDISTRIBUTION_HPP_

#include <iostream>
#include <fstream>
#include <vector>

namespace Ace {

class AngularDistribution {

public:

	friend class ANDBlock;

	typedef int TableType;
	/* Type of scattering cosine tables */
	const static TableType isotropic_table = 0;
	const static TableType equibins_table = 0;
	const static TableType tabular_table = 0;

	/* Array where the distribution is defined (32 equiprobable bins, tabular or isotropic) */
	struct AngularArray {
		AngularArray() {/* */};
		virtual void dump(std::ostream& out) const = 0;
		virtual int size() const = 0;
		virtual AngularArray* clone() const = 0;
		virtual TableType getType() const = 0;
		virtual ~AngularArray() {/* */};
	};

	/* If LC(J)=0, isotropic and no further information is needed. */
	struct Isotropic : public AngularArray {
		Isotropic() {/* */};
		void dump(std::ostream& out) const {/**/};
		int size() const {return 0;};
		TableType getType() const {return isotropic_table;};
		virtual AngularArray* clone() const {return new Isotropic();};
	};

	/* If LC(J) is positive, it points to a 32 equiprobable bin distribution. */
	struct EquiBins : public AngularArray {
		std::vector<double> bins;
		EquiBins(const std::vector<double>& bins) : bins(bins) {/* */};
		void dump(std::ostream& out) const;
		int size() const {return bins.size();};
		TableType getType() const {return equibins_table;};
		virtual AngularArray* clone() const {return new EquiBins(bins);};
	};

	/* If LC(J) is negative, it points to a tabular angular distribution. */
	struct Tabular : public AngularArray {
		int iflag;                 /* 1 = histogram, 2 = lin-lin */
		std::vector<double> csout; /* Cosine scattering angular grid */
		std::vector<double> pdf;   /* Probability density function */
		std::vector<double> cdf;   /* Cumulative density function */
		Tabular(int iflag, const std::vector<double>& csout,const std::vector<double>& pdf,const std::vector<double>& cdf) :
			    iflag(iflag), csout(csout), pdf(pdf), cdf(cdf) {/* */};
		void dump(std::ostream& out) const;
		int size() const {return (3 * csout.size() + 2);};
		TableType getType() const {return tabular_table;};
		virtual AngularArray* clone() const {return new Tabular(iflag,csout,pdf,cdf);};
	};

	void dump(std::ostream& xss) const;

	int getSize() const;

	int getKind() const {return kind;};

	/*
	 * No angular distribution data are given for this reaction,
	 * and isotropic scattering is assumed in either the LAB or CM system.
	 */
	static const int isotropic = 0;

	/*
	 * No angular distribution data are given for this reaction in the AND Block. Angular
	 * distribution data are specified through LAW=44 in the DLW Block.
	 */
	static const int law44 = 1;

	/*
	 * There is data on the object
	 */
	static const int data = 2;

	/*
	 * No data at all
	 */
	static const int no_data = 3;

	AngularDistribution(const std::vector<double>& energy,const std::vector<int>& lc,const std::vector<AngularArray*>& adist) :
		                energy(energy),lc(lc),adist(adist),kind(data) {/* */};
	AngularDistribution(int kind) : energy(0),lc(0),adist(0),kind(kind) {/* */}
	AngularDistribution(const AngularDistribution& ad);

	virtual ~AngularDistribution();

	/* Energies at which the distributions are tabulated */
	std::vector<double> energy;
	/* Location of energy distributions (relative to JXS(9)) */
	std::vector<int> lc;
	/* Array of angular distribution for each energy */
	std::vector<AngularArray*> adist;
	/* Kind of data included on the distribution */
	int kind;

private:

	AngularDistribution(std::vector<double>::const_iterator it);

};

} /* namespace ACE */
#endif /* ANGULARDISTRIBUTION_HPP_ */
