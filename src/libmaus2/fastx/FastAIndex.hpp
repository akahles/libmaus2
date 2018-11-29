/*
    libmaus2
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
#if ! defined(LIBMAUS2_FASTX_FASTAINDEX_HPP)
#define LIBMAUS2_FASTX_FASTAINDEX_HPP

#include <libmaus2/autoarray/AutoArray.hpp>
#include <libmaus2/fastx/FastAIndexEntry.hpp>
#include <libmaus2/util/stringFunctions.hpp>
#include <libmaus2/exception/LibMausException.hpp>
#include <libmaus2/aio/InputStreamFactoryContainer.hpp>

#include <vector>
#include <sstream>

namespace libmaus2
{
	namespace fastx
	{
		struct FastAIndex
		{
			typedef FastAIndex this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

			std::vector<libmaus2::fastx::FastAIndexEntry> sequences;
			std::map<std::string,uint64_t> shortNameToId;

			uint64_t serialise(std::ostream & out) const
			{
				uint64_t s = 0;

				s += libmaus2::util::NumberSerialisation::serialiseNumber(out,sequences.size());
				for ( uint64_t i = 0; i < sequences.size(); ++i )
					s += sequences[i].serialise(out);
				s += libmaus2::util::NumberSerialisation::serialiseNumber(out,shortNameToId.size());
				for ( std::map<std::string,uint64_t>::const_iterator ita = shortNameToId.begin(); ita != shortNameToId.end(); ++ita )
				{
					s += libmaus2::util::StringSerialisation::serialiseString(out,ita->first);
					s += libmaus2::util::NumberSerialisation::serialiseNumber(out,ita->second);
				}
				return s;
			}

			void deserialise(std::istream & in)
			{
				sequences.resize(0);
				uint64_t const nseq = libmaus2::util::NumberSerialisation::deserialiseNumber(in);
				for ( uint64_t i = 0; i < nseq; ++i )
					sequences.push_back(libmaus2::fastx::FastAIndexEntry(in));
				uint64_t const nshort = libmaus2::util::NumberSerialisation::deserialiseNumber(in);
				shortNameToId.clear();
				for ( uint64_t i = 0; i < nshort; ++i )
				{
					std::string const key = libmaus2::util::StringSerialisation::deserialiseString(in);
					uint64_t const val = libmaus2::util::NumberSerialisation::deserialiseNumber(in);
					shortNameToId[key] = val;
				}
			}

			static unique_ptr_type loadSerialised(std::istream & in)
			{
				unique_ptr_type tptr(new this_type);
				tptr->deserialise(in);
				return tptr;
			}

			static unique_ptr_type loadSerialised(std::string const & fn)
			{
				libmaus2::aio::InputStreamInstance ISI(fn);
				unique_ptr_type tptr(loadSerialised(ISI));
				return tptr;
			}

			FastAIndex() : sequences()
			{

			}

			std::vector<libmaus2::fastx::FastAIndexEntry>::size_type size() const
			{
				return sequences.size();
			}

			libmaus2::fastx::FastAIndexEntry operator[](std::vector<libmaus2::fastx::FastAIndexEntry>::size_type const i) const
			{
				return sequences.at(i);
			}

			uint64_t getSequenceIdByName(std::string const & s) const
			{
				std::string const shortname = computeShortName(s);

				if ( shortNameToId.find(shortname) != shortNameToId.end() )
					return shortNameToId.find(shortname)->second;

				libmaus2::exception::LibMausException lme;
				lme.getStream() << "libmaus2::fastx::FastAIndex::getSequenceIdByName(" << s << "): sequence is not in database\n";
				lme.finish();
				throw lme;
			}

			static std::string computeShortName(std::string const & s)
			{
				uint64_t i = 0;
				while (
					i < s.size() &&
					(!isspace(static_cast<unsigned char>(s[i])))
				)
					++i;

				return s.substr(0,i);
			}

			static uint64_t parseNumber(std::string const & s)
			{
				std::istringstream istr(s);
				uint64_t n;
				istr >> n;

				if ( ! istr )
				{
					libmaus2::exception::LibMausException lme;
					lme.getStream() << "FastAIndexEntry::parseNumber(): cannot parse " << s << " as number" << std::endl;
					lme.finish();
					throw lme;
				}

				return n;
			}

			FastAIndex(std::istream & in) : sequences()
			{
				while ( in )
				{
					std::string line;
					std::getline(in,line);

					if ( line.size() )
					{
						std::deque<std::string> tokens = libmaus2::util::stringFunctions::tokenize<std::string>(line,std::string("\t"));

						if ( tokens.size() >= 5 )
						{
							shortNameToId[computeShortName(tokens[0])] = sequences.size();

							sequences.push_back(
								libmaus2::fastx::FastAIndexEntry(
									tokens[0],
									parseNumber(tokens[1]),
									parseNumber(tokens[2]),
									parseNumber(tokens[3]),
									parseNumber(tokens[4])
								)
							);
						}
					}
				}
			}

			static unique_ptr_type load(std::string const & filename)
			{
				libmaus2::aio::InputStream::unique_ptr_type PCIS(
					libmaus2::aio::InputStreamFactoryContainer::constructUnique(filename)
				);
				unique_ptr_type tptr(new this_type(*PCIS));
				return tptr;
			}

			libmaus2::autoarray::AutoArray<char> readSequence(std::istream & in, int64_t const seqid) const
			{
				if ( seqid < 0 || seqid >= static_cast<int64_t>(sequences.size()) )
				{
					libmaus2::exception::LibMausException lme;
					lme.getStream() << "FastAIndexEntry::readSequence(): sequence id " << seqid << " is out of range" << std::endl;
					lme.finish();
					throw lme;
				}

				libmaus2::fastx::FastAIndexEntry const entry = sequences.at(seqid);
				uint64_t const lineskip = entry.bytesperline-entry.basesperline;

				libmaus2::autoarray::AutoArray<char> A(entry.length,false);
				char * cur = A.begin();

				uint64_t todo = entry.length;

				in.clear();
				in.seekg(entry.offset,std::ios::beg);

				while ( todo )
				{
					uint64_t const re = std::min(todo,entry.basesperline);

					in.read(cur,re);

					if ( in.gcount() != static_cast<int64_t>(re) )
					{
						libmaus2::exception::LibMausException lme;
						lme.getStream() << "FastAIndexEntry::readSequence(): failed to read sequence " << entry.name << std::endl;
						lme.finish();
						throw lme;
					}

					cur += re;
					todo -= re;

					if ( todo )
					{
						in.ignore(lineskip);

						if ( in.gcount() != static_cast<int64_t>(lineskip) )
						{
							libmaus2::exception::LibMausException lme;
							lme.getStream() << "FastAIndexEntry::readSequence(): failed to read sequence " << entry.name << std::endl;
							lme.finish();
							throw lme;
						}
					}
				}

				return A;
			}

			libmaus2::autoarray::AutoArray<char> readSequenceRange(std::istream & in, int64_t const seqid, int64_t const low, int64_t const high) const
			{
				if ( seqid < 0 || seqid >= static_cast<int64_t>(sequences.size()) )
				{
					libmaus2::exception::LibMausException lme;
					lme.getStream() << "FastAIndexEntry::readSequenceRange(): sequence id " << seqid << " is out of range" << std::endl;
					lme.finish();
					throw lme;
				}

				libmaus2::fastx::FastAIndexEntry const entry = sequences.at(seqid);

				if ( low < 0 || high > static_cast<int64_t>(entry.length) || low > high )
				{
					libmaus2::exception::LibMausException lme;
					lme.getStream() << "FastAIndexEntry::readSequenceRange(): invalid range" << std::endl;
					lme.finish();
					throw lme;
				}

				uint64_t const lineskip = entry.bytesperline-entry.basesperline;

				uint64_t todo = high-low;
				libmaus2::autoarray::AutoArray<char> A(todo,false);
				char * cur = A.begin();

				// seek to position on sequence
				in.clear();
				in.seekg(entry.offset + ( low / entry.basesperline ) * entry.bytesperline + (low % entry.basesperline), std::ios::beg);

				// if we are not at a line start, then read to (at most) end of line
				if ( (low % entry.basesperline) != 0 )
				{
					uint64_t const re = std::min(todo,entry.basesperline - (low % entry.basesperline));

					in.read(cur,re);

					if ( in.gcount() != static_cast<int64_t>(re) )
					{
						libmaus2::exception::LibMausException lme;
						lme.getStream() << "FastAIndexEntry::readSequenceRange(): failed to read sequence " << entry.name << std::endl;
						lme.finish();
						throw lme;
					}

					cur += re;
					todo -= re;

					if ( todo )
					{
						in.ignore(lineskip);

						if ( in.gcount() != static_cast<int64_t>(lineskip) )
						{
							libmaus2::exception::LibMausException lme;
							lme.getStream() << "FastAIndexEntry::readSequenceRange(): failed to read sequence " << entry.name << std::endl;
							lme.finish();
							throw lme;
						}
					}
				}

				// read rest
				while ( todo )
				{
					uint64_t const re = std::min(todo,entry.basesperline);

					in.read(cur,re);

					if ( in.gcount() != static_cast<int64_t>(re) )
					{
						libmaus2::exception::LibMausException lme;
						lme.getStream() << "FastAIndexEntry::readSequenceRange(): failed to read sequence " << entry.name << std::endl;
						lme.finish();
						throw lme;
					}

					cur += re;
					todo -= re;

					if ( todo )
					{
						in.ignore(lineskip);

						if ( in.gcount() != static_cast<int64_t>(lineskip) )
						{
							libmaus2::exception::LibMausException lme;
							lme.getStream() << "FastAIndexEntry::readSequenceRange(): failed to read sequence " << entry.name << std::endl;
							lme.finish();
							throw lme;
						}
					}
				}

				assert ( cur == A.end() );

				return A;
			}
		};

		std::ostream & operator<<(std::ostream & out, FastAIndex const & index);
	}
}
#endif
