/*
    libmaus2
    Copyright (C) 2009-2015 German Tischler
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
#if ! defined(LIBMAUS2_LCP_WAVELETLCP_HPP)
#define LIBMAUS2_LCP_WAVELETLCP_HPP

#include <libmaus2/bitio/CompactQueue.hpp>
#include <libmaus2/lcp/WaveletLCPResult.hpp>
#include <libmaus2/timing/RealTimeClock.hpp>
#include <libmaus2/math/ilog.hpp>
#include <stack>

#if defined(_OPENMP)
#include <omp.h>
#endif

namespace libmaus2
{
	namespace lcp
	{
		/**
		 * LCP array construction algorithm as described by Beller et al
		 **/
		struct WaveletLCP
		{
			template<typename lf_type>
			static WaveletLCPResult::unique_ptr_type computeLCP(
				lf_type const * LF,
				uint64_t const rnumthreads,
				bool const zdif,
				std::ostream * logstr
			)
			{
				typename lf_type::D_type const & D = LF->getD();

				uint64_t const n = LF->getN();
				WaveletLCPResult::small_elem_type const unset = std::numeric_limits< WaveletLCPResult::small_elem_type>::max();
				WaveletLCPResult::unique_ptr_type res(new WaveletLCPResult(n));

				::libmaus2::autoarray::AutoArray< WaveletLCPResult::small_elem_type> & WLCP = *(res->WLCP);
				::libmaus2::autoarray::AutoArray<uint64_t> symfreq( LF->getSymbolThres() );

				if ( logstr )
					*logstr << "[V] symbol threshold is " << symfreq.size() << std::endl;

				// symbol frequencies
				for ( uint64_t i = 0; i < symfreq.getN(); ++i )
				{
					symfreq[i] = n?(LF->getW().rankm(i,n)):0;
					// std::cerr << "sym " << i << " freq " << symfreq[i] << std::endl;
				}

				std::fill (WLCP.begin(),WLCP.begin()+n,unset);

				::libmaus2::suffixsort::CompactQueue Q0(n);
				::libmaus2::suffixsort::CompactQueue Q1(n);
				::libmaus2::suffixsort::CompactQueue * PQ0 = &Q0;
				::libmaus2::suffixsort::CompactQueue * PQ1 = &Q1;

				uint64_t s = 0;
				uint64_t cc = 0;
				uint64_t acc = 0;

				if ( zdif )
				{
					// special handling of zero symbols
					for ( uint64_t zc = 0; zc < symfreq[0]; ++zc )
					{
						WLCP[zc] = 0;
						Q0.enque(zc,zc+1);
					}
					s += symfreq[cc++];
					acc += symfreq[0];
				}

				// other symbols
				for ( ; cc < symfreq.getN(); ++cc )
				{
					WLCP[acc] = 0;
					s++;

					if ( symfreq[cc] )
					{
						Q0.enque(acc,acc+symfreq[cc]);
						acc += symfreq[cc];
					}
				}
				WLCP[n] = 0;

				::libmaus2::timing::RealTimeClock lcprtc; lcprtc.start();
				if ( logstr )
					*logstr << "[V] Computing LCP...";
				uint64_t cur_l = 1;
				uint64_t maxfill = 0;
				uint64_t const logn = std::max ( static_cast<int64_t> ( libmaus2::math::ilog(n) ), static_cast<int64_t>(1) );
				while (
					PQ0->fill && cur_l < unset
				)
				{
					if ( logstr )
						*logstr << "(" << static_cast<uint64_t>(cur_l) << "," << PQ0->fill << "," << s << "(" << static_cast<double>(s)/n << "))";

					PQ1->reset();

					#if defined(LIBMAUS2_HAVE_SYNC_OPS)
					uint64_t const numthreads = rnumthreads;
					#else
					uint64_t const numthreads = 1;
					#endif

					uint64_t const numcontexts = numthreads;
					::libmaus2::autoarray::AutoArray < ::libmaus2::suffixsort::CompactQueue::DequeContext::unique_ptr_type > deqcontexts = PQ0->getContextList(numcontexts,numthreads);

					#if defined(_OPENMP) && defined(LIBMAUS2_HAVE_SYNC_OPS)
					#pragma omp parallel for num_threads(numthreads)
					#endif
					for ( int64_t c = 0; c < static_cast<int64_t>(deqcontexts.size()); ++c )
					{
						::libmaus2::suffixsort::CompactQueue::DequeContext * deqcontext = deqcontexts[c].get();
						::libmaus2::suffixsort::CompactQueue::EnqueBuffer::unique_ptr_type encbuf = PQ1->createEnqueBuffer();

						while ( !deqcontext->done() )
						{
							std::pair<uint64_t,uint64_t> const qe = PQ0->deque(deqcontext);

							uint64_t const locals = (LF->getW()).multiRankLCPSet(qe.first,qe.second,D.get(),WLCP.get(),unset,cur_l,encbuf.get());
							#if defined(_OPENMP) && defined(LIBMAUS2_HAVE_SYNC_OPS)
							__sync_fetch_and_add(&s,locals);
							#else
							s += locals;
							#endif
						}

						assert ( deqcontext->fill == 0 );
					}
					std::swap(PQ0,PQ1);

					cur_l ++;

					if ( PQ0->fill > maxfill )
						maxfill = PQ0->fill;
					if ( PQ0->fill < maxfill && s >= n / logn && (PQ0->fill < n / logn) )
						break;
				}

				// extract compact queues into non compact ones
				std::deque< std::pair<uint64_t,uint64_t> > DQ0, DQ1;

				if ( PQ0->fill )
				{
					// assert ( cur_l == unset );

					::libmaus2::suffixsort::CompactQueue::DequeContext::unique_ptr_type dcontext = PQ0->getGlobalDequeContext();
					while ( dcontext->fill )
						DQ0.push_back( PQ0->deque(dcontext.get()) );
				}

				if ( DQ0.size() )
				{
					// unset == largest value for the small type
					uint64_t prefill = 0;

					while ( DQ0.size() )
					{
						if ( cur_l == unset )
						{
							// prepare result for storing "large" values
							res->setupLargeValueVector(n, unset);
						}

						if ( DQ0.size() != prefill )
						{
							if ( logstr )
								*logstr << "(" << static_cast<uint64_t>(cur_l) << "," << DQ0.size() << "," << s << " (" << static_cast<double>(s)/n << "))";
							prefill = DQ0.size();
						}

						assert ( DQ1.size() == 0 );

						if ( cur_l >= unset )
							while ( DQ0.size() )
							{
								std::pair<uint64_t,uint64_t> const qe = DQ0.front(); DQ0.pop_front();
								uint64_t const locals = (LF->getW()).multiRankLCPSetLarge(qe.first,qe.second,D.get(),*res,cur_l,&DQ1);
								#if defined(_OPENMP) && defined(LIBMAUS2_HAVE_SYNC_OPS)
								__sync_fetch_and_add(&s,locals);
								#else
								s += locals;
								#endif
							}
						else
							while ( DQ0.size() )
							{
								std::pair<uint64_t,uint64_t> const qe = DQ0.front(); DQ0.pop_front();
								uint64_t const locals = (LF->getW()).multiRankLCPSet(qe.first,qe.second,D.get(),WLCP.get(),unset,cur_l,&DQ1);
								#if defined(_OPENMP) && defined(LIBMAUS2_HAVE_SYNC_OPS)
								__sync_fetch_and_add(&s,locals);
								#else
								s += locals;
								#endif
							}


						assert ( ! DQ0.size() );
						DQ0.swap(DQ1);

						cur_l ++;
					}

				}

				if ( logstr )
					*logstr << " done, time " << lcprtc.getElapsedSeconds() << std::endl;

				return res;
			}
		};


	}
}
#endif
