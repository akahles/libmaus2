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
#if ! defined(LIBMAUS2_AIO_OUTPUTSTREAMINSTANCE_HPP)
#define LIBMAUS2_AIO_OUTPUTSTREAMINSTANCE_HPP

#include <libmaus2/aio/OutputStreamPointerWrapper.hpp>
#include <ostream>

namespace libmaus2
{
	namespace aio
	{
		struct OutputStreamInstance : protected OutputStreamPointerWrapper, public std::ostream
		{
			typedef OutputStreamInstance this_type;
			typedef std::unique_ptr<this_type> unique_ptr_type;
			typedef std::shared_ptr<this_type> shared_ptr_type;

			OutputStreamInstance(std::string const & fn);
		};
	}
}
#endif
