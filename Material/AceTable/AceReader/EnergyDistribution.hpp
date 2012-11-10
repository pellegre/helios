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

#ifndef ENERGYDISTRIBUTION_HPP_
#define ENERGYDISTRIBUTION_HPP_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>

#include "AngularDistribution.hpp"

namespace Ace {

class EnergyDistribution {

public:

	static std::map<int,std::string> energy_laws;

	/* ENDF interpolation */
	struct InterScheme {
		int nr;                       /* Number of interpolation regions */
		std::vector<int> nbt;         /* ENDF interpolation parameters */
		std::vector<int> aint;
		InterScheme() { /* */ };
		InterScheme(int nr,const std::vector<int>& nbt,const std::vector<int>&aint) : nr(nr),nbt(nbt),aint(aint) {/* */};
		InterScheme(std::vector<double>::const_iterator& it);
		void dump(std::ostream& xss) const;
		int getSize() const;
		~InterScheme() {/**/};
	};

    class EnergyLaw {

		/* Factory of laws */
		static EnergyLaw* createLaw(std::vector<double>::const_iterator it, int lnw, int law);

		/* Clone the object */
		virtual EnergyLaw* clone() const = 0;

		/* Dump Law array */
		virtual void dump_array(std::ostream& xss) const = 0;
		/* Size of the law array */
		virtual int size_array() const = 0;

	protected:

		/* Common data to all energy laws */
		int lnw;                      /* Location of next law */
		int idat;                     /* Location of this data */
		InterScheme endf_int;
		int ne;                       /* Number of energies */
		std::vector<double> energy;   /* Tabular energies points */
		std::vector<double> prob;     /* Probability law validity */

		/* Set the law array */
		void set_common(std::vector<double>::const_iterator& it);

		EnergyLaw(std::vector<double>::const_iterator& it, int lnw);

	public:

		EnergyLaw(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob) :
			      lnw(lnw),idat(idat),endf_int(nr,nbt,aint),ne(ne),energy(energy),prob(prob) {/* */};

		friend class EnergyDistribution;

		/* Number representing this law */
		virtual int getLaw() const = 0;
		/* Name of the law */
		virtual std::string getLawName() const = 0;

		/* Dump this law */
		void dump(std::ostream& xss) const;
		/* Get the total size */
		int getSize() const;

		virtual ~EnergyLaw() {/* */};

	};

    /* Tabular equiprobable energy bins */
    class Law1 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law1(std::vector<double>::const_iterator it, int lnw);

		InterScheme int_sch;
        int nen;
        std::vector<double> ein;
        int net;
        std::vector<std::vector<double> > eout;

    public:

        Law1(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 1; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[1]; };

        ~Law1() {/* */};

    };
    /* Discrete photon energy */
    class Law2 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law2(std::vector<double>::const_iterator it, int lnw);

        int lp;
        double eg;

    public:

        Law2(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 2; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[2]; };

        ~Law2() {/* */};

    };
    /* Level scattering */
    class Law3 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law3(std::vector<double>::const_iterator it, int lnw);

        double ldat1;
        double ldat2;

    public:

        Law3(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 3; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[3]; };

        ~Law3() {/* */};

    };
    /* Continuous tabular distribution */
    class Law4 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law4(std::vector<double>::const_iterator it, int lnw);

    public:

        struct EnergyData {
        	int intt;
        	int np;
            std::vector<double> eout;
            std::vector<double> pdf;
            std::vector<double> cdf;
            int getSize() const {return (2 + eout.size() + pdf.size() + cdf.size());};
        };

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<int> loca;
        std::vector<EnergyData> eout_dist;

        Law4(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 4; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[4]; };

        ~Law4() {/* */};

    };
    /* General evaporation spectrum */
    class Law5 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law5(std::vector<double>::const_iterator it, int lnw);

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<double> t;
        int net;
        std::vector<double> x;

    public:

        Law5(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 5; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[5]; };

        ~Law5() {/* */};

    };
    /* Simple Maxwell fission spectrum */
    class Law7 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law7(std::vector<double>::const_iterator it, int lnw);

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<double> t;
        double u;

    public:

        Law7(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 7; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[7]; };

        ~Law7() {/* */};

    };
    /* Evaporation spectrum */
    class Law9 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law9(std::vector<double>::const_iterator it, int lnw);

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<double> t;
        double u;

    public:

        Law9(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 9; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[9]; };

        ~Law9() {/* */};

    };
    /* Energy dependent Watt spectrum */
    class Law11 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law11(std::vector<double>::const_iterator it, int lnw);

		InterScheme inta;
		int nena;
        std::vector<double> eina;
        std::vector<double> a;

		InterScheme intb;
		int nenb;
        std::vector<double> einb;
        std::vector<double> b;
        double u;

    public:

        Law11(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 11; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[11]; };

        ~Law11() {/* */};

    };
    /* Tabular linear functions */
    class Law22 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law22(std::vector<double>::const_iterator it, int lnw);

        struct EnergyData {
        	int nf;
        	std::vector<double> p;
        	std::vector<double> t;
        	std::vector<double> c;
        	int getSize() const {return (1 + p.size() + t.size() + c.size());};
        };

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<int> loca;
        std::vector<EnergyData> eout_dist;

    public:

        Law22(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 22; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[22]; };

        ~Law22() {/* */};

    };
    /* From UK Law 6 */
    class Law24 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law24(std::vector<double>::const_iterator it, int lnw);

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        int net;
        std::vector<std::vector<double> > t;

    public:

        Law24(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 24; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[24]; };

        ~Law24() {/* */};

    };
    /* Kalbach-87 Formalism */
    class Law44 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law44(std::vector<double>::const_iterator it, int lnw);

        struct EnergyData {
        	int intt;
        	int np;
            std::vector<double> eout;
            std::vector<double> pdf;
            std::vector<double> cdf;
            std::vector<double> r;
            std::vector<double> a;
            int getSize() const {return (2 + eout.size() + pdf.size() + cdf.size() + r.size() + a.size());};
        };

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<int> loca;
        std::vector<EnergyData> eout_dist;

    public:

        Law44(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 44; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[44]; };

        ~Law44() {/* */};

    };
    /* Like 44, but tabular distribution instead of Kalbach-87 */
    class Law61 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law61(std::vector<double>::const_iterator it, int lnw);

        struct EnergyData {
        	int intt;
            std::vector<double> eout;
            std::vector<double> pdf;
            std::vector<double> cdf;
            std::vector<int> lc;
            std::vector<AngularDistribution::AngularArray*> adist;

            int getSize() const;
        	void dump(std::ostream& xss) const;

        	EnergyData(std::vector<double>::const_iterator it);
            EnergyData(const EnergyData& ed);

            ~EnergyData();
        };

		InterScheme int_sch;
		int nen;
        std::vector<double> ein;
        std::vector<int> loca;
        std::vector<EnergyData> eout_dist;

    public:

        Law61(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 61; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[61]; };

        ~Law61() {/* */};

    };
    /* N-body phase space distribution */
    class Law66 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law66(std::vector<double>::const_iterator it, int lnw);

        double npxs;
        double ap;

    public:

        Law66(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 66; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[66]; };

        ~Law66() {/* */};

    };
    /* Laboratory angle-energy law */
    class Law67 : public EnergyLaw {

        void dump_array(std::ostream& xss) const;

        int size_array() const;

        EnergyLaw* clone() const;

        Law67(std::vector<double>::const_iterator it, int lnw);

    public:

        Law67(int lnw,int idat,int nr,const std::vector<int>& nbt,const std::vector<int>&aint,int ne,const std::vector<double>& energy,const std::vector<double>& prob)
                : EnergyLaw(lnw,idat,nr,nbt,aint,ne,energy,prob) {/* */};

        friend class EnergyDistribution;

        int getLaw() const { return 67; };

        std::string getLawName() const { return EnergyDistribution::energy_laws[67]; };

        ~Law67() {/* */};

    };

	int getKind() const {return kind;};

	/* Return the laws of this energy distribution */
	std::vector<int> getLaws() const;

	/* Container of energy laws */
	std::vector<EnergyLaw*> laws;

	/*
	 * There is data on the object
	 */
	static const int data = 0;

	/*
	 * No data at all
	 */
	static const int no_data = 1;

	EnergyDistribution(const EnergyDistribution& ed);
	EnergyDistribution(int kind) : laws(0),kind(kind) {/* */}

	/* Dump all the Laws representing the energy distribution */
	void dump(std::ostream& xss) const;
	/* Get the size of this energy distribution */
	int getSize() const;

	virtual ~EnergyDistribution();

private:

	friend class DLWBlock;

	EnergyDistribution(std::vector<double>::const_iterator it, std::vector<double>::const_iterator begin);

	/* Kind of data included on the distribution */
	int kind;
};

} /* namespace ACE */

#endif /* ENERGYDISTRIBUTION_HPP_ */
