/*
    libmaus2
    Copyright (C) 2015 German Tischler

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
#if ! defined(LIBMAUS2_FASTX_DNAINDEXMETADATABIGBAND_HPP)
#define LIBMAUS2_FASTX_DNAINDEXMETADATABIGBAND_HPP

#include <libmaus2/fastx/DNAIndexMetaDataSequence.hpp>
#include <libmaus2/math/numbits.hpp>
#include <libmaus2/math/lowbits.hpp>
#include <libmaus2/util/PrefixSums.hpp>

namespace libmaus2
{
	namespace fastx
	{
		struct DNAIndexMetaDataBigBand
		{
			typedef DNAIndexMetaDataBigBand this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

			std::vector<DNAIndexMetaDataSequence> S;
			std::vector<uint64_t> L;
			uint64_t maxl;

			DNAIndexMetaDataBigBand(std::istream & in)
			{
				uint64_t const numseq = libmaus2::util::NumberSerialisation::deserialiseNumber(in);
				S.resize(numseq);
				for ( uint64_t i = 0; i < numseq; ++i )
					S[i] = DNAIndexMetaDataSequence(in);
				L.resize(numseq+1);
				for ( uint64_t i = 0; i < numseq; ++i )
					L[i] = S[i].l;
				libmaus2::util::PrefixSums::prefixSums(L.begin(),L.end());

				maxl = 0;
				for ( uint64_t i = 0; i < S.size(); ++i )
					maxl = std::max(maxl,S[i].l);
			}

			static unique_ptr_type load(std::istream & in)
			{
				unique_ptr_type tptr(new this_type(in));
				return tptr;
			}

			static unique_ptr_type load(std::string const & s)
			{
				libmaus2::aio::InputStreamInstance ISI(s);
				unique_ptr_type tptr(new this_type(ISI));
				return tptr;
			}

			std::pair<uint64_t,uint64_t> mapCoordinates(uint64_t const i) const
			{
				assert ( i < L.back() );
				std::vector<uint64_t>::const_iterator ita = std::lower_bound(L.begin(),L.end(),i);
				assert ( ita != L.end() );

				std::pair<uint64_t,uint64_t> R;

				if ( i == *ita )
					R = std::pair<uint64_t,uint64_t>(ita-L.begin(),0);
				else
				{
					ita -= 1;
					assert ( *ita <= i );
					R = std::pair<uint64_t,uint64_t>(ita-L.begin(),i - *ita);
				}

				assert ( L[R.first] + R.second == i );

				return R;
			}

			bool valid(std::pair<uint64_t,uint64_t> const & P, uint64_t const k) const
			{
				uint64_t const seqlen = S[P.first].l;
				return P.second + k <= seqlen;
			}
		};

		std::ostream & operator<<(std::ostream & out, DNAIndexMetaDataBigBand const & D);
	}
}
#endif
