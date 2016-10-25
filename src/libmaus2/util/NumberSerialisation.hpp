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

#if ! defined(NUMBERSERIALISATION_HPP)
#define NUMBERSERIALISATION_HPP

#include <ostream>
#include <istream>
#include <stdexcept>
#include <deque>
#include <set>
#include <iomanip>
#include <libmaus2/types/types.hpp>
#include <libmaus2/exception/LibMausException.hpp>
#include <libmaus2/math/DoubleCode.hpp>

namespace libmaus2
{
	namespace util
	{
		struct NumberSerialisation
		{

			static std::string formatNumber(uint64_t const n, unsigned int const w)
			{
				std::ostringstream ostr;
				ostr << std::setw(w) << std::setfill('0') << n;
				return ostr.str();
			}

			template<typename stream_type>
			static uint64_t serialiseNumber(stream_type & out, uint64_t const n, uint64_t const k)
			{
				for ( uint64_t i = 0; i < k; ++i )
					out.put ((n >> (k-i-1)*8) & 0xFF );

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber()";
					se.finish();
					throw se;
				}

				return k;
			}

			template<typename stream_type>
			static uint64_t serialiseNumber16(stream_type & out, uint16_t const n, uint64_t const k)
			{
				for ( uint64_t i = 0; i < k; ++i )
					out.put ((n >> (k-i-1)*8) & 0xFF );

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber16()";
					se.finish();
					throw se;
				}

				return k;
			}

            template<typename stream_type>
			static uint64_t serialiseNumber32(stream_type & out, uint32_t const n, uint64_t const k)
			{
				for ( uint64_t i = 0; i < k; ++i )
					out.put ((n >> (k-i-1)*8) & 0xFF );

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber32()";
					se.finish();
					throw se;
				}

				return k;
			}


			template<typename stream_type>
			static uint64_t serialiseNumber(stream_type & out, uint64_t const n)
			{
				out.put ( (n >>  (7*8)) & 0xFF);
				out.put ( (n >>  (6*8)) & 0xFF);
				out.put ( (n >>  (5*8)) & 0xFF);
				out.put ( (n >>  (4*8)) & 0xFF);
				out.put ( (n >>  (3*8)) & 0xFF);
				out.put ( (n >>  (2*8)) & 0xFF);
				out.put ( (n >>  (1*8)) & 0xFF);
				out.put ( (n >>  (0*8)) & 0xFF);

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber()";
					se.finish();
					throw se;
				}

				return 8;
			}

			template<typename stream_type>
			static uint64_t serialiseNumber16(stream_type & out, uint16_t const n)
			{
				out.put ( (n >>  (1*8)) & 0xFF);
				out.put ( (n >>  (0*8)) & 0xFF);

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber16()";
					se.finish();
					throw se;
				}

				return 2;
			}

			template<typename stream_type>
			static uint64_t serialiseNumber32(stream_type & out, uint32_t const n)
			{
				out.put ( (n >>  (3*8)) & 0xFF);
				out.put ( (n >>  (2*8)) & 0xFF);
				out.put ( (n >>  (1*8)) & 0xFF);
				out.put ( (n >>  (0*8)) & 0xFF);

				if ( ! out )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "failure in ::libmaus2::util::NumberSerialisation::serialiseNumber32()";
					se.finish();
					throw se;
				}

				return 4;
			}



			template<typename N>
			static uint64_t serialiseNumberVector(std::ostream & out, std::vector<N> const & V)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, V.size());

				for ( uint64_t i = 0; i < V.size(); ++i )
					s += serialiseNumber(out, V[i]);

				return s;
			}

			template<typename N>
			static uint64_t serialiseNumber16Vector(std::ostream & out, std::vector<N> const & V)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, V.size());

				for ( uint64_t i = 0; i < V.size(); ++i )
					s += serialiseNumber16(out, V[i]);

				return s;
			}

			template<typename N>
			static uint64_t serialiseNumber32Vector(std::ostream & out, std::vector<N> const & V)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, V.size());

				for ( uint64_t i = 0; i < V.size(); ++i )
					s += serialiseNumber32(out, V[i]);

				return s;
			}

			static uint64_t serialiseNumberSet(std::ostream & out, std::set<uint64_t> const & S)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, S.size());

                for (std::set<uint64_t> :: iterator it = S.begin(); it != S.end(); ++it)
                    s += serialiseNumber(out, *it); 

				return s;
			}


			static uint64_t serialiseNumber16Set(std::ostream & out, std::set<uint16_t> const & S)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, S.size());

                for (std::set<uint16_t> :: iterator it = S.begin(); it != S.end(); ++it)
                    s += serialiseNumber16(out, *it); 

				return s;
			}

			static uint64_t serialiseNumber32Set(std::ostream & out, std::set<uint32_t> const & S)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, S.size());

                for (std::set<uint32_t> :: iterator it = S.begin(); it != S.end(); ++it)
                    s += serialiseNumber32(out, *it); 

				return s;
			}




			template<typename N>
			static uint64_t serialiseNumberDeque(std::ostream & out, std::deque<N> const & D)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, D.size());

				for ( uint64_t i = 0; i < D.size(); ++i )
					s += serialiseNumber(out, D[i]);

				return s;
			}

			template<typename N>
			static uint64_t serialiseNumber16Deque(std::ostream & out, std::deque<N> const & D)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, D.size());

				for ( uint64_t i = 0; i < D.size(); ++i )
					s += serialiseNumber16(out, D[i]);

				return s;
			}

			template<typename N>
			static uint64_t serialiseNumber32Deque(std::ostream & out, std::deque<N> const & D)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, D.size());

				for ( uint64_t i = 0; i < D.size(); ++i )
					s += serialiseNumber32(out, D[i]);

				return s;
			}


			template<typename stream_type>
			static uint64_t deserialiseNumber(stream_type & in, uint64_t const k)
			{
				uint64_t v = 0;

				for ( uint64_t i = 0; i < k; ++i )
				{
					int const c = in.get();
					if ( c < 0 )
					{
                    	::libmaus2::exception::LibMausException se;
                    	se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber()";
                    	se.finish();
                    	throw se;
					}
					uint64_t const u = c;
					v <<= 8;
					v |= u;
				}

				return v;
			}

			template<typename stream_type>
			static uint16_t deserialiseNumber16(stream_type & in, uint64_t const k)
			{
				uint16_t v = 0;

				for ( uint64_t i = 0; i < k; ++i )
				{
					int const c = in.get();
					if ( c < 0 )
					{
                    	::libmaus2::exception::LibMausException se;
                    	se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber16()";
                    	se.finish();
                    	throw se;
					}
					uint16_t const u = c;
					v <<= 8;
					v |= u;
				}

				return v;
			}

			template<typename stream_type>
			static uint32_t deserialiseNumber32(stream_type & in, uint64_t const k)
			{
				uint32_t v = 0;

				for ( uint64_t i = 0; i < k; ++i )
				{
					int const c = in.get();
					if ( c < 0 )
					{
                    	::libmaus2::exception::LibMausException se;
                    	se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber32()";
                    	se.finish();
                    	throw se;
					}
					uint32_t const u = c;
					v <<= 8;
					v |= u;
				}

				return v;
			}



			template<typename stream_type>
			static uint64_t deserialiseNumber(stream_type & in)
			{
				int const c0 = in.get();
				int const c1 = in.get();
				int const c2 = in.get();
				int const c3 = in.get();
				int const c4 = in.get();
				int const c5 = in.get();
				int const c6 = in.get();
				int const c7 = in.get();

				if (  c0 < 0 || c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0 || c5 < 0 || c6 < 0 || c7 < 0 )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber()";
					se.finish();
					throw se;
				}

				uint64_t const u0 = c0;
				uint64_t const u1 = c1;
				uint64_t const u2 = c2;
				uint64_t const u3 = c3;
				uint64_t const u4 = c4;
				uint64_t const u5 = c5;
				uint64_t const u6 = c6;
				uint64_t const u7 = c7;

				uint64_t const u =
					  (u0 << (7*8))
					| (u1 << (6*8))
					| (u2 << (5*8))
					| (u3 << (4*8))
					| (u4 << (3*8))
					| (u5 << (2*8))
					| (u6 << (1*8))
					| (u7 << (0*8))
					;

				return u;
			}

			template<typename stream_type>
			static uint16_t deserialiseNumber16(stream_type & in)
			{
				int const c0 = in.get();
				int const c1 = in.get();

				if (  c0 < 0 || c1 < 0 )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber16()";
					se.finish();
					throw se;
				}

				uint16_t const u0 = c0;
				uint16_t const u1 = c1;

				uint16_t const u =
					  (u0 << (1*8))
					| (u1 << (0*8))
					;

				return u;
			}

			template<typename stream_type>
			static uint32_t deserialiseNumber32(stream_type & in)
			{
				int const c0 = in.get();
				int const c1 = in.get();
				int const c2 = in.get();
				int const c3 = in.get();

				if (  c0 < 0 || c1 < 0 || c2 < 0 || c3 < 0 )
				{
					::libmaus2::exception::LibMausException se;
					se.getStream() << "EOF/failure in ::libmaus2::util::NumberSerialisation::deserialiseNumber32()";
					se.finish();
					throw se;
				}

				uint32_t const u0 = c0;
				uint32_t const u1 = c1;
				uint32_t const u2 = c2;
				uint32_t const u3 = c3;

				uint32_t const u =
					  (u0 << (3*8))
					| (u1 << (2*8))
					| (u2 << (1*8))
					| (u3 << (0*8))
					;

				return u;
			}



			template<typename stream_type>
			static uint64_t deserialiseNumberCount(stream_type & in, uint64_t & s)
			{
				s += 8;
				return deserialiseNumber(in);
			}

			template<typename N>
			static std::vector<N> deserialiseNumberVector(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::vector < N > V;

				for ( uint64_t i = 0; i < n; ++i )
					V.push_back(deserialiseNumber(in));

				return V;
			}

			template<typename N>
			static std::vector<N> deserialiseNumber16Vector(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::vector < N > V;

				for ( uint64_t i = 0; i < n; ++i )
					V.push_back(deserialiseNumber16(in));

				return V;
			}

			template<typename N>
			static std::vector<N> deserialiseNumber32Vector(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::vector < N > V;

				for ( uint64_t i = 0; i < n; ++i )
					V.push_back(deserialiseNumber32(in));

				return V;
			}

            template<typename N>
			static std::set<N> deserialiseNumberSet(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::set < N > S;

				for ( uint64_t i = 0; i < n; ++i )
					S.insert(deserialiseNumber(in));

				return S;
			}

			template<typename N>
			static std::set<N> deserialiseNumber16Set(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::set < N > S;

				for ( uint64_t i = 0; i < n; ++i )
					S.insert(deserialiseNumber16(in));

				return S;
			}

			template<typename N>
			static std::set<N> deserialiseNumber32Set(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::set < N > S;

				for ( uint64_t i = 0; i < n; ++i )
					S.insert(deserialiseNumber32(in));

				return S;
			}


			template<typename N>
			static std::deque<N> deserialiseNumberDeque(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::deque < N > D;

				for ( uint64_t i = 0; i < n; ++i )
					D.push_back(deserialiseNumber(in));

				return D;
			}

			template<typename N>
			static std::deque<N> deserialiseNumber16Deque(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::deque < N > D;

				for ( uint64_t i = 0; i < n; ++i )
					D.push_back(deserialiseNumber16(in));

				return D;
			}

			template<typename N>
			static std::deque<N> deserialiseNumber32Deque(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::deque < N > D;

				for ( uint64_t i = 0; i < n; ++i )
					D.push_back(deserialiseNumber32(in));

				return D;
			}



			template<typename stream_type>
			static uint64_t serialiseDouble(stream_type & out, double const d)
			{
				serialiseNumber(out,::libmaus2::math::DoubleCode::encodeDouble(d));
				return 8;
			}

			template<typename stream_type>
			static double deserialiseDouble(stream_type & in)
			{
				return ::libmaus2::math::DoubleCode::decodeDouble(deserialiseNumber(in));
			}

			static uint64_t serialiseDoubleVector(std::ostream & out, std::vector<double> const & V)
			{
				uint64_t  s = 0;
				s += serialiseNumber(out, V.size());

				for ( uint64_t i = 0; i < V.size(); ++i )
					s += serialiseDouble(out, V[i]);

				return s;
			}

			static std::vector<double> deserialiseDoubleVector(std::istream & in)
			{
				uint64_t const n = deserialiseNumber(in);
				std::vector < double > V;

				for ( uint64_t i = 0; i < n; ++i )
					V.push_back(deserialiseDouble(in));

				return V;
			}

			static unsigned int countBytes(uint64_t n)
			{
				unsigned int c = 0;

				if ( n > 0xFFFFFFFFull ) { c += 4; n >>= (4*8); } assert ( n <= 0xFFFFFFFFull );
				if ( n > 0xFFFFull     ) { c += 2; n >>= (2*8); } assert ( n <= 0xFFFFull     );
				if ( n > 0xFFull       ) { c += 1; n >>= (1*8); } assert ( n <= 0xFFull       );
				if ( n                 ) { c += 1; n >>= (1*8); } assert ( n == 0 );

				return c;
			}

			template<typename put_type>
			static uint64_t serialiseSignedNumber(put_type & P, int64_t const m)
			{
				uint8_t f;
				unsigned int b;
				uint64_t n;

				if ( m < 0 )
				{
					n = -m;
					b = countBytes(n);
					// f = 0x80 | (8-b);
					f = (8-b);

					// std::cerr << "m=" << m << " n=" << n << " b=" << b << std::endl;

					switch ( b )
					{
						case 0: break;
						case 1: assert (n <=               0xffull ); n =               0xffull-n; break;
						case 2: assert (n <=             0xffffull ); n =             0xffffull-n; break;
						case 3: assert (n <=           0xffffffull ); n =           0xffffffull-n; break;
						case 4: assert (n <=         0xffffffffull ); n =         0xffffffffull-n; break;
						case 5: assert (n <=       0xffffffffffull ); n =       0xffffffffffull-n; break;
						case 6: assert (n <=     0xffffffffffffull ); n =     0xffffffffffffull-n; break;
						case 7: assert (n <=   0xffffffffffffffull ); n =   0xffffffffffffffull-n; break;
						case 8: assert (n <= 0xffffffffffffffffull ); n = 0xffffffffffffffffull-n; break;
					}
				}
				else
				{
					n = m;
					b = countBytes(n);
					// f = b;
					f = 0x80 | b;
				}

				unsigned int o = 1;
				// put sign bit and length of number
				P.put(f);
				// put number
				for ( int ib = (static_cast<int>(b)-1); ib >= 0; --ib )
				{
					P.put( (n >> (8*ib)) & 0xFF );
					o += 1;
				}

				return o;
			}

			template<typename get_type>
			static int64_t deserialiseSignedNumber(get_type & G)
			{
				uint8_t const f = G.get();
				uint8_t b;

				// positive
				if ( f & 0xf0 )
				{
					b =    f & 0x7F ;
				}
				// negative
				else
				{
					b = 8-(f & 0x7F);
				}

				uint64_t n = 0;

				for ( unsigned int ib = 0; ib < b; ++ib )
				{
					n <<= 8;
					n |= G.get();
				}

				if ( f & 0xf0 )
				{
					return n;
				}
				else
				{
					switch ( b )
					{
						case 0: break;
						case 1: assert (n <=               0xffull ); n =               0xffull-n; break;
						case 2: assert (n <=             0xffffull ); n =             0xffffull-n; break;
						case 3: assert (n <=           0xffffffull ); n =           0xffffffull-n; break;
						case 4: assert (n <=         0xffffffffull ); n =         0xffffffffull-n; break;
						case 5: assert (n <=       0xffffffffffull ); n =       0xffffffffffull-n; break;
						case 6: assert (n <=     0xffffffffffffull ); n =     0xffffffffffffull-n; break;
						case 7: assert (n <=   0xffffffffffffffull ); n =   0xffffffffffffffull-n; break;
						case 8: assert (n <= 0xffffffffffffffffull ); n = 0xffffffffffffffffull-n; break;
					}

					return - static_cast<int64_t>(n);
				}
			}

			template<typename get_type>
			static int64_t deserialiseSignedNumberCount(get_type & G, uint64_t & s)
			{
				uint8_t const f = G.get();
				s += 1;
				uint8_t b;

				// positive
				if ( f & 0xf0 )
				{
					b =    f & 0x7F ;
				}
				// negative
				else
				{
					b = 8-(f & 0x7F);
				}

				uint64_t n = 0;

				for ( unsigned int ib = 0; ib < b; ++ib )
				{
					n <<= 8;
					n |= G.get();
					s += 1;
				}

				if ( f & 0xf0 )
				{
					return n;
				}
				else
				{
					switch ( b )
					{
						case 0: break;
						case 1: assert (n <=               0xffull ); n =               0xffull-n; break;
						case 2: assert (n <=             0xffffull ); n =             0xffffull-n; break;
						case 3: assert (n <=           0xffffffull ); n =           0xffffffull-n; break;
						case 4: assert (n <=         0xffffffffull ); n =         0xffffffffull-n; break;
						case 5: assert (n <=       0xffffffffffull ); n =       0xffffffffffull-n; break;
						case 6: assert (n <=     0xffffffffffffull ); n =     0xffffffffffffull-n; break;
						case 7: assert (n <=   0xffffffffffffffull ); n =   0xffffffffffffffull-n; break;
						case 8: assert (n <= 0xffffffffffffffffull ); n = 0xffffffffffffffffull-n; break;
					}

					return - static_cast<int64_t>(n);
				}
			}

			static int64_t recodeSignedNumber(int64_t const n)
			{
				std::ostringstream ostr;
				::libmaus2::util::NumberSerialisation::serialiseSignedNumber(ostr,n);
				std::istringstream istr(ostr.str());
				return ::libmaus2::util::NumberSerialisation::deserialiseSignedNumber(istr);
			}

			virtual ~NumberSerialisation() {}
		};
	}
}
#endif
