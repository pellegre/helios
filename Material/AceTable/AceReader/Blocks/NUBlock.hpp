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

#ifndef NUBLOCK_HPP_
#define NUBLOCK_HPP_

#include "ACEBlock.hpp"

namespace Ace {

	class NUBlock: public AceTable::ACEBlock {

	public:

		/* Flag of type of data */
		static const int flag_pol =  1;
		static const int flag_tab =  2;

		class NuData : public AceTable::ACEBlock {
		public:
			NuData() {/* */}
			NuData(std::vector<double>::const_iterator _it, AceTable* ace_table) : ACEBlock(_it,ace_table) {/* */};
			virtual void dump(std::ostream& xss) = 0;
			virtual int getSize() const = 0;
			virtual int getType() const = 0;
			static std::string name() {return "NUData";}
			std::string blockName() const {return name();};
			virtual NuData* clone() const = 0;
			virtual ~NuData() {/* */};
		};

		struct Polynomial : public NuData {
			Polynomial(int ncoef, const std::vector<double>& coef) : ncoef(ncoef), coef(coef) {/* */}
		public:
			/* Polynomial function form of NU array */
			int ncoef;                 /* Number of coefficients */
			std::vector<double> coef;  /* Coefficients */
			Polynomial(std::vector<double>::const_iterator _it, AceTable* ace_table);
			void dump(std::ostream& xss);
			int getSize() const {return (1 + coef.size());};
			int getType() const {return flag_pol;};
			NuData* clone() const {return new Polynomial(ncoef, coef);};
			~Polynomial() {/* */};
		};

		class Tabular : public NuData {
			Tabular(int nr, const std::vector<int>& nbt, const std::vector<int>& aint,
					const std::vector<double>& energies, const std::vector<double>& nu) : nr(nr), nbt(nbt), aint(aint),
					ne(energies.size()), energies(energies), nu(nu) {/* */}
		public:
			/* Tabular data form of NU array */
			int nr;                       /* Number of interpolation regions */
			std::vector<int> nbt;         /* ENDF interpolation parameters */
			std::vector<int> aint;
			int ne;                       /* Number of energies */
			std::vector<double> energies; /* tabular energies points */
			std::vector<double> nu;       /* Values of NU */
			Tabular(std::vector<double>::const_iterator _it, AceTable* ace_table);
			void dump(std::ostream& xss);
			int getSize() const {return (2 + nbt.size() + aint.size() + energies.size() + nu.size());};
			int getType() const {return flag_tab;};
			NuData* clone() const {return new Tabular(nr, nbt, aint, energies, nu);};
			~Tabular() {/* */};
		};

		friend class NeutronTable;

		/* Dump the block, on a xss stream */
		void dump(std::ostream& xss);

		int getSize() const;

		int getType() const;

		static std::string name() {return "NUBlock";}
		std::string blockName() const {return name();};

		/* Clone data inside this block */
		std::vector<NuData*> clone() const;

		virtual ~NUBlock();

	private:

		/* Containers of NU data */
		std::vector<NuData*> nu_data; /* This one can have total NU data too. */

		NUBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, AceTable* ace_table);

	};

} /* namespace ACE */
#endif /* NUBLOCK_HPP_ */
