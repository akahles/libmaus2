/*
    libmaus2
    Copyright (C) 2017 German Tischler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if !defined(LIBMAUS2_LCS_NNPCORL_HPP)
#define LIBMAUS2_LCS_NNPCORL_HPP

#include <libmaus2/lcs/NNPCor.hpp>
#include <libmaus2/lcs/NPL.hpp>
#include <libmaus2/lcs/NP.hpp>

namespace libmaus2
{
	namespace lcs
	{
		struct NNPCorL : public libmaus2::lcs::AlignmentTraceContainer
		{
			typedef NNPCorL this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

			libmaus2::lcs::NNPCor cor;
			NNPTraceContainer tracecontainer;

			libmaus2::lcs::NPL o_npl;
			libmaus2::lcs::NP o_np;

			NNPCorL(
				double const mincor = NNPCor::getDefaultMinCorrelation(),
				double const minlocalcor = NNPCor::getDefaultMinLocalCorrelation(),
				int64_t const rmaxback = NNPCor::getDefaultMaxBack(),
				bool const rfuniquetermval = NNPCor::getDefaultUniqueTermVal(),
				bool const rrunsuffixpositive = NNPCor::getDefaultRunSuffixPositive()
			) : cor(mincor,minlocalcor,rmaxback,rfuniquetermval,rrunsuffixpositive) {}

			void reset()
			{
				cor.reset();
			}

			template<typename iterator>
                        std::pair<uint64_t,uint64_t> alignForward(
                                iterator ab,
                                iterator ae,
                                iterator bb,
                                iterator be,
                                bool const self = false,
                                bool const uniquetermval = false,
                                int64_t const minband = NNPCor::getDefaultMinDiag(),
                                int64_t const maxband = NNPCor::getDefaultMaxDiag()
                        )
                        {
                        	std::pair<uint64_t,uint64_t> const SL =
                        		cor.alignForward(
                        			ab,ae,bb,be,
                        			tracecontainer,
                        			*this,
                        			self,
                        			uniquetermval,
                        			minband,
                        			maxband
                        		);
				return SL;
                        }

			template<typename iterator>
                        std::pair<uint64_t,uint64_t> np(
                                iterator ab,
                                iterator ae,
                                iterator bb,
                                iterator be,
                                bool const self = false,
                                bool const uniquetermval = false,
                                int64_t const minband = NNPCor::getDefaultMinDiag(),
                                int64_t const maxband = NNPCor::getDefaultMaxDiag()
                        )
                        {
                        	std::pair<uint64_t,uint64_t> SL = alignForward(ab,ae,bb,be,self,uniquetermval,minband,maxband);

                        	uint64_t const n_a = ae-ab;
                        	uint64_t const n_b = be-bb;

                        	if ( SL.first != n_a || SL.second != n_b )
                        	{
                        		o_np.np(
                        			ab + SL.first, ae,
                        			bb + SL.second,be
                        		);

                        		push(o_np);

                        		std::pair<uint64_t,uint64_t> SL_O = o_np.getStringLengthUsed();
                        		SL.first  += SL_O.first;
                        		SL.second += SL_O.second;

                        		o_np.cutTrace(100);
                        	}

                        	return SL;
                        }

			template<typename iterator>
                        std::pair<uint64_t,uint64_t> npl(
                                iterator ab,
                                iterator ae,
                                iterator bb,
                                iterator be,
                                bool const self = false,
                                bool const uniquetermval = false,
                                int64_t const minband = NNPCor::getDefaultMinDiag(),
                                int64_t const maxband = NNPCor::getDefaultMaxDiag()
                        )
                        {
                        	std::pair<uint64_t,uint64_t> SL = alignForward(ab,ae,bb,be,self,uniquetermval,minband,maxband);

                        	uint64_t const n_a = ae-ab;
                        	uint64_t const n_b = be-bb;

                        	if ( SL.first != n_a && SL.second != n_b )
                        	{
                        		o_npl.np(
                        			ab + SL.first, ae,
                        			bb + SL.second,be
                        		);

                        		push(o_npl);

                        		std::pair<uint64_t,uint64_t> SL_O = o_npl.getStringLengthUsed();
                        		SL.first  += SL_O.first;
                        		SL.second += SL_O.second;

                        		o_npl.cutTrace(100);
                        	}

                        	return SL;
                        }

                        libmaus2::lcs::AlignmentTraceContainer const & getTraceContainer() const
                        {
                        	return *this;
                        }
		};
	}
}
#endif
