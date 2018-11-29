/*
    libmaus2
    Copyright (C) 2009-2013 German Tischler
    Copyright (C) 2011-2013 Genome Research Limited

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

#if ! defined(LIBMAUS2_EXCEPTION_LIBMAUSEXCEPTION_HPP)
#define LIBMAUS2_EXCEPTION_LIBMAUSEXCEPTION_HPP

#include <memory>
#include <sstream>
#include <libmaus2/util/StackTrace.hpp>

namespace libmaus2
{
	namespace exception
	{
		struct LibMausException : std::exception, ::libmaus2::util::StackTrace
		{
			typedef LibMausException this_type;
			typedef std::unique_ptr<this_type> unique_ptr_type;

            std::shared_ptr<std::ostringstream> postr;
			std::string s;

			LibMausException()
			: postr(new std::ostringstream), s()
			{

			}
			~LibMausException() throw()
			{

			}

			unique_ptr_type uclone() const
			{
				unique_ptr_type uptr(new this_type);

				uptr->postr = std::shared_ptr<std::ostringstream>(new std::ostringstream(postr->str()));
				uptr->s = s;

				return uptr;
			}

			static unique_ptr_type uclone(std::exception const & ex)
			{
				unique_ptr_type Tptr(new this_type);
				Tptr->getStream() << ex.what() << std::endl;
				Tptr->finish();
				return Tptr;
			}

			std::ostream & getStream()
			{
				return *postr;
			}

			void finish(bool translateStackTrace = true)
			{
				s = postr->str();
				s += "\n";
				s += ::libmaus2::util::StackTrace::toString(translateStackTrace);
			}

			char const * what() const throw()
			{
				return s.c_str();
			}
		};
	}
}
#endif
