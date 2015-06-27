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
#if ! defined(LIBMAUS2_LCS_ALIGNERFACTORY_HPP)
#define LIBMAUS2_LCS_ALIGNERFACTORY_HPP

#include <libmaus2/lcs/Aligner.hpp>

namespace libmaus2
{
	namespace lcs
	{
		struct AlignerFactory
		{
			enum aligner_type {
				libmaus2_lcs_AlignerFactory_x128_8,
				libmaus2_lcs_AlignerFactory_x128_16,
				libmaus2_lcs_AlignerFactory_y256_8,
				libmaus2_lcs_AlignerFactory_y256_16
			};
			
			static libmaus2::lcs::Aligner::unique_ptr_type construct(aligner_type const type);
		};
	}
}
#endif
