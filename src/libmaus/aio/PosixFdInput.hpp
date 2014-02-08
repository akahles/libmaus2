/*
    libmaus
    Copyright (C) 2009-2014 German Tischler
    Copyright (C) 2011-2014 Genome Research Limited

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
#if ! defined(LIBMAUS_AIO_POSIXFDINPUT_HPP)
#define LIBMAUS_AIO_POSIXFDINPUT_HPP

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <libmaus/exception/LibMausException.hpp>
#include <libmaus/lz/StreamWrapper.hpp>

namespace libmaus
{
	namespace aio
	{
		struct PosixFdInput
		{
			public:
			typedef PosixFdInput this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
			
			private:
			std::string filename;
			int fd;
			ssize_t gcnt;
			
			public:
			static int getDefaultFlags()
			{
				return O_RDONLY|O_NOATIME|O_LARGEFILE;
			}
			
			PosixFdInput(int const rfd) : fd(rfd)
			{
			}
			
			PosixFdInput(std::string const & rfilename, int const rflags = getDefaultFlags())
			: filename(rfilename), fd(-1), gcnt(0)
			{
				while ( fd == -1 )
				{
					fd = ::open(filename.c_str(),rflags);
					
					if ( fd < 0 )
					{
						switch ( errno )
						{
							case EINTR:
							case EAGAIN:
								break;
							default:
							{
								int const error = errno;
								libmaus::exception::LibMausException se;
								se.getStream() << "PosixFdInput(" << filename << "," << rflags << "): " << strerror(error) << std::endl;
								se.finish();
								throw se;
							}
						}
					}
				}
			}
			
			ssize_t read(char * const buffer, uint64_t const n)
			{
				ssize_t r = -1;
				gcnt = 0;

				while ( fd >= 0 && r < 0 )
				{
					r = ::read(fd,buffer,n);
					
					if ( r < 0 )
					{
						switch ( errno )
						{
							case EINTR:
							case EAGAIN:
								break;
							default:
							{
								int const error = errno;
								libmaus::exception::LibMausException se;
								se.getStream() << "PosixFdInput::read(" << filename << "," << n << "): " << strerror(error) << std::endl;
								se.finish();
								throw se;
							}
						}
					}
					else
					{
						gcnt = r;
					}
				}

				return gcnt;
			}
			
			ssize_t gcount() const
			{
				return gcnt;
			}
			
			void close()
			{	
				int r = -1;
				
				while ( fd >= 0 && r == -1 )
				{
					r = ::close(fd);
					
					if ( r < 0 )
					{
						switch ( errno )
						{
							case EINTR:
								break;
							default:
							{
								int const error = errno;
								libmaus::exception::LibMausException se;
								se.getStream() << "PosixFdInput::close(" << filename << "): " << strerror(error) << std::endl;
								se.finish();
								throw se;
							}
						}
					}
					else
					{
						fd = -1;
					}
				}
			}
		};
	}
}
#endif
