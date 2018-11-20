/*
    libmaus2
    Copyright (C) 2009-2015 German Tischler
    Copyright (C) 2011-2015 Genome Research Limited

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
#if ! defined(LIBMAUS2_AIO_INPUTOUTPUTSTREAMWRAPPER_HPP)
#define LIBMAUS2_AIO_INPUTOUTPUTSTREAMWRAPPER_HPP

#include <memory>
#include <iostream>

namespace libmaus2
{
	namespace aio
	{
		struct InputOutputStreamWrapper
		{
			typedef InputOutputStreamWrapper this_type;
			typedef std::unique_ptr<this_type> unique_ptr_type;
			typedef std::shared_ptr<this_type> shared_ptr_type;

			typedef std::iostream stream_type;
			typedef std::shared_ptr<stream_type> shared_stream_ptr_type;

			shared_stream_ptr_type Sstream;
			stream_type & stream;

			InputOutputStreamWrapper(shared_stream_ptr_type & Tstream) : Sstream(Tstream), stream(*Sstream) {}
			InputOutputStreamWrapper(stream_type & rstream)            : Sstream(),        stream(rstream) {}
			virtual ~InputOutputStreamWrapper() {}

			stream_type & getStream()
			{
				return stream;
			}
		};
	}
}
#endif
