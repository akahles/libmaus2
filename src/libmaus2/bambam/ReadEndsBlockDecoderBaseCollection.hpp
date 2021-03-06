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
#if ! defined(LIBMAUS2_BAMBAM_READENDSBLOCKDECODERBASECOLLECTION_HPP)
#define LIBMAUS2_BAMBAM_READENDSBLOCKDECODERBASECOLLECTION_HPP

#include <libmaus2/autoarray/AutoArray.hpp>
#include <libmaus2/bambam/BamAlignment.hpp>
#include <libmaus2/bambam/CompactReadEndsBase.hpp>
#include <libmaus2/bambam/CompactReadEndsComparator.hpp>
#include <libmaus2/bambam/ReadEnds.hpp>
#include <libmaus2/bambam/ReadEndsBlockDecoderBase.hpp>
#include <libmaus2/bambam/ReadEndsBlockDecoderBaseCollectionInfo.hpp>
#include <libmaus2/bambam/ReadEndsContainerBase.hpp>
#include <libmaus2/bambam/ReadEndsHeapPairComparator.hpp>
#include <libmaus2/bambam/SortedFragDecoder.hpp>
#include <libmaus2/index/ExternalMemoryIndexGenerator.hpp>
#include <libmaus2/lz/SnappyOutputStream.hpp>
#include <libmaus2/sorting/SerialRadixSort64.hpp>
#include <libmaus2/timing/RealTimeClock.hpp>
#include <libmaus2/util/DigitTable.hpp>
#include <libmaus2/util/CountGetObject.hpp>
#include <libmaus2/util/UnsignedIntegerIndexIterator.hpp>
#include <libmaus2/aio/InputStreamFactoryContainer.hpp>

namespace libmaus2
{
	namespace bambam
	{
		template<bool _proxy>
		struct ReadEndsBlockDecoderBaseCollection
		{
			static bool const proxy = _proxy;
			typedef ReadEndsBlockDecoderBaseCollection<proxy> this_type;
			typedef typename libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef typename libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

			struct ReadEndsBlockDecoderBaseCollectionCountSmallerShortAccessor
			{
				ReadEndsBlockDecoderBaseCollection<proxy> const * object;

				ReadEndsBlockDecoderBaseCollectionCountSmallerShortAccessor(ReadEndsBlockDecoderBaseCollection<proxy> * robject = 0) : object(robject) {}

				uint64_t get(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
				{
					return object->countSmallerThanShort(H);
				}
			};

			struct ReadEndsBlockDecoderBaseCollectionCountSmallerLongAccessor
			{
				ReadEndsBlockDecoderBaseCollection<proxy> const * object;

				ReadEndsBlockDecoderBaseCollectionCountSmallerLongAccessor(ReadEndsBlockDecoderBaseCollection<proxy> * robject = 0) : object(robject) {}

				uint64_t get(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
				{
					return object->countSmallerThanLong(H);
				}
			};

			std::vector<ReadEndsBlockDecoderBaseCollectionInfo> info;
			uint64_t const numblocks;

			libmaus2::autoarray::AutoArray< typename ::libmaus2::bambam::ReadEndsBlockDecoderBase<proxy>::unique_ptr_type> Adecoders;

			ReadEndsBlockDecoderBaseCollectionCountSmallerShortAccessor const countShortAccessor;
			ReadEndsBlockDecoderBaseCollectionCountSmallerLongAccessor const countLongAccessor;

			static std::vector<ReadEndsBlockDecoderBaseCollectionInfo> constructInfo(
				std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & rinfo,
				bool const parallelSetup,
				uint64_t const
					#if defined(_OPENMP)
					numthreads
					#endif
			)
			{
				std::vector<ReadEndsBlockDecoderBaseCollectionInfo> info(rinfo.size());

				if ( parallelSetup )
				{
					#if defined(_OPENMP)
					#pragma omp parallel for num_threads(numthreads)
					#endif
					for ( uint64_t i = 0; i < rinfo.size(); ++i )
						info[i] = ReadEndsBlockDecoderBaseCollectionInfo(rinfo[i]);
				}
				else
				{
					for ( uint64_t i = 0; i < rinfo.size(); ++i )
						info[i] = ReadEndsBlockDecoderBaseCollectionInfo(rinfo[i]);
				}

				return info;
			}

			ReadEndsBlockDecoderBaseCollection(
				std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & rinfo,
				bool const parallelSetup,
				uint64_t const numthreads
			) : info(constructInfo(rinfo,parallelSetup,numthreads)),
			    numblocks(computeNumBlocks()),
			    Adecoders(numblocks,false),
			    countShortAccessor(this),
			    countLongAccessor(this)
			{
				uint64_t j = 0;
				for ( uint64_t k = 0; k < info.size(); ++k )
				{
					ReadEndsBlockDecoderBaseCollectionInfo & subinfo = info[k];

					for ( uint64_t i = 0; i < subinfo.indexoffset.size(); ++i )
					{
						typename libmaus2::bambam::ReadEndsBlockDecoderBase<proxy>::unique_ptr_type tptr(
							new libmaus2::bambam::ReadEndsBlockDecoderBase<proxy>(
								subinfo,
								subinfo,
								*(subinfo.datastr),
								*(subinfo.indexstr),
								subinfo.indexoffset.at(i),
								subinfo.blockelcnt.at(i)
							)
						);

						Adecoders.at(j++) = UNIQUE_PTR_MOVE(tptr);
					}
				}
			}

			uint64_t computeNumBlocks() const
			{
				uint64_t s = 0;
				for ( uint64_t i = 0; i < info.size(); ++i )
					s += info[i].indexoffset.size();
				return s;
			}

			uint64_t size() const
			{
				return numblocks;
			}

			::libmaus2::bambam::ReadEndsBlockDecoderBase<proxy> const * getBlock(uint64_t const i) const
			{
				return Adecoders[i].get();
			}

			uint64_t totalEntries() const
			{
				uint64_t sum = 0;
				for ( uint64_t i = 0; i < size(); ++i )
					sum += getBlock(i)->size();
				return sum;
			}

			ReadEnds max() const
			{
				ReadEnds RE;
				for ( uint64_t i = 0; i < size(); ++i )
					if ( getBlock(i) && getBlock(i)->size() )
						RE = std::max(RE,getBlock(i)->get(getBlock(i)->size()-1));
				return RE;
			}

			uint64_t countSmallerThanShort(ReadEndsBase const & A) const
			{
				uint64_t sum = 0;
				for ( uint64_t i = 0; i < size(); ++i )
					sum += getBlock(i)->countSmallerThanShort(A);
				return sum;
			}

			uint64_t countSmallerThanLong(ReadEndsBase const & A) const
			{
				uint64_t sum = 0;
				for ( uint64_t i = 0; i < size(); ++i )
					sum += getBlock(i)->countSmallerThanLong(A);
				return sum;
			}

			uint64_t countSmallerThanShort(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
			{
				ReadEndsBase B;
				B.decodeShortHash(H);
				return countSmallerThanShort(B);
			}

			uint64_t countSmallerThanLong(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
			{
				ReadEndsBase B;
				B.decodeLongHash(H);
				return countSmallerThanLong(B);
			}

			static libmaus2::bambam::ReadEndsBase::hash_value_type hashLowerBound()
			{
				return libmaus2::bambam::ReadEndsBase::hash_value_type(0);
			}

			libmaus2::bambam::ReadEndsBase::hash_value_type hashUpperBoundShort() const
			{
				return max().encodeShortHash() + libmaus2::bambam::ReadEndsBase::hash_value_type(1);
			}

			libmaus2::bambam::ReadEndsBase::hash_value_type hashUpperBoundLong() const
			{
				return max().encodeLongHash() + libmaus2::bambam::ReadEndsBase::hash_value_type(1);
			}

			typedef libmaus2::util::UnsignedIntegerIndexIterator<ReadEndsBlockDecoderBaseCollectionCountSmallerShortAccessor,uint64_t,libmaus2::bambam::ReadEndsBase::hash_value_words>
				outer_short_iterator;
			typedef libmaus2::util::UnsignedIntegerIndexIterator<ReadEndsBlockDecoderBaseCollectionCountSmallerLongAccessor,uint64_t,libmaus2::bambam::ReadEndsBase::hash_value_words>
				outer_long_iterator;

			outer_short_iterator outerShortBegin() const
			{
				return outer_short_iterator(&countShortAccessor,hashLowerBound());
			}

			outer_short_iterator outerShortEnd() const
			{
				return outer_short_iterator(&countShortAccessor,hashUpperBoundShort());
			}

			outer_long_iterator outerLongBegin() const
			{
				return outer_long_iterator(&countLongAccessor,hashLowerBound());
			}

			outer_long_iterator outerLongEnd() const
			{
				return outer_long_iterator(&countLongAccessor,hashUpperBoundLong());
			}

			std::vector < std::vector< std::pair<uint64_t,uint64_t> > > getShortMergeIntervals(uint64_t const numthreads, bool const check = false)
			{
				std::vector < std::vector< std::pair<uint64_t,uint64_t> > > R(numthreads,std::vector< std::pair<uint64_t,uint64_t> >(size()));
				uint64_t const totalentries = totalEntries();
				uint64_t const targetfrac = totalentries/numthreads;
				std::vector< ::libmaus2::bambam::ReadEnds::hash_value_type > splitPoints(numthreads);

				for ( uint64_t i = 0; i < numthreads; ++i )
				{
					splitPoints[i] = std::lower_bound(outerShortBegin(),outerShortEnd(),i * targetfrac).i;
					::libmaus2::bambam::ReadEnds RA;
					RA.decodeShortHash(splitPoints[i]);

					for ( uint64_t j = 0; j < size(); ++j )
						R[i][j].first = getBlock(j)->countSmallerThanShort(RA);
				}

				for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					for ( uint64_t j = 0; j < size(); ++j )
						R[i][j].second = R[i+1][j].first;

				if ( numthreads )
					for ( uint64_t j = 0; j < size(); ++j )
						R[numthreads-1][j].second = getBlock(j)->size();

				if ( check )
				{
					for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					{
						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[i];
						::libmaus2::bambam::ReadEnds::hash_value_type const splithigh = splitPoints[i+1];

						for ( uint64_t j = 0; j < size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[i][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( getBlock(j)->get(k).encodeShortHash() >= splitlow );
								assert ( getBlock(j)->get(k).encodeShortHash() < splithigh );
							}
						}
					}

					if ( numthreads )
					{
						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[numthreads-1];

						for ( uint64_t j = 0; j < size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[numthreads-1][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( getBlock(j)->get(k).encodeShortHash() >= splitlow );
							}
						}
					}
				}

				return R;
			}

			static uint64_t getTotalEntries(std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & info)
			{
				uint64_t s = 0;
				for ( uint64_t k = 0; k < info.size(); ++k )
				{
					ReadEndsBlockDecoderBaseCollectionInfo const & subinfo = info[k];

					for ( uint64_t i = 0; i < subinfo.blockelcnt.size(); ++i )
						s += subinfo.blockelcnt.at(i);
				}

				return s;
			}

			static uint64_t computeNumBlocks(std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & info)
			{
				uint64_t s = 0;
				for ( uint64_t i = 0; i < info.size(); ++i )
					s += info[i].indexoffset.size();
				return s;
			}

			typedef libmaus2::index::ExternalMemoryIndexDecoder<
				libmaus2::bambam::ReadEndsBase,
				libmaus2::bambam::ReadEndsContainerBase::baseIndexShift,
				libmaus2::bambam::ReadEndsContainerBase::innerIndexShift,
				libmaus2::bambam::ReadEndsBaseShortHashAttributeComparator
			> short_index_decoder_type;
			typedef short_index_decoder_type::unique_ptr_type short_index_decoder_pointer_type;

			static uint64_t getTotalShortBlocks(libmaus2::autoarray::AutoArray<short_index_decoder_pointer_type> const & A)
			{
				uint64_t s = 0;
				for ( uint64_t i = 0; i < A.size(); ++i )
					s += A[i]->size();
				return s;
			}

			struct ExternalMemoryIndexDecoderShortCountAccessor
			{
				typedef ExternalMemoryIndexDecoderShortCountAccessor this_type;
				typedef typename libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
				typedef typename libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

				libmaus2::autoarray::AutoArray<short_index_decoder_pointer_type> & Pdecoders;

				ExternalMemoryIndexDecoderShortCountAccessor(
					libmaus2::autoarray::AutoArray<short_index_decoder_pointer_type> & Rdecoders
				) : Pdecoders(Rdecoders) {}

				/**
				 * get maximum block start element
				 **/
				libmaus2::bambam::ReadEndsBase max()
				{
					libmaus2::bambam::ReadEndsBase R;
					R.reset();

					for ( uint64_t i = 0; i < Pdecoders.size(); ++i )
						if ( Pdecoders[i]->maxelvalid && R.encodeShortHash() < Pdecoders[i]->maxel.encodeShortHash() )
						{
							R = Pdecoders[i]->maxel;
							R.decodeShortHash(R.encodeShortHash());
						}

					return R;
				}

				uint64_t get(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
				{
					libmaus2::bambam::ReadEndsBase R;
					R.decodeShortHash(H);

					uint64_t c = 0;
					for ( uint64_t i = 0; i < Pdecoders.size(); ++i )
						c += Pdecoders[i]->findLargestSmaller(R,true /* cache only */).blockid;

					return c;
				}
			};

			typedef libmaus2::util::UnsignedIntegerIndexIterator<ExternalMemoryIndexDecoderShortCountAccessor,uint64_t,libmaus2::bambam::ReadEndsBase::hash_value_words>
				short_index_iterator;

			static short_index_iterator indexShortBegin(ExternalMemoryIndexDecoderShortCountAccessor & raccessor)
			{
				return short_index_iterator(&raccessor,hashLowerBound());
			}

			static short_index_iterator indexShortEnd  (ExternalMemoryIndexDecoderShortCountAccessor & raccessor)
			{
				return short_index_iterator(&raccessor,raccessor.max().encodeShortHash()+libmaus2::bambam::ReadEndsBase::hash_value_type(1));
			}

			struct UVal
			{
				uint64_t volatile v;

				UVal() : v(0) {}
				UVal(uint64_t const rv) : v(rv) {}
			};

			static std::vector < std::vector< std::pair<uint64_t,uint64_t> > > getShortMergeIntervals(
				std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & rinfo,
				uint64_t const numthreads,
				bool const check = false
			)
			{
				#if defined(READENDSBLOCKTIMING)
				libmaus2::timing::RealTimeClock decodersetuprtc;
				#endif
				uint64_t const memory = 256ull*1024ull*1024ull;
				uint64_t const numblocks = computeNumBlocks(rinfo);
				uint64_t const minmem = 16*1024;
				uint64_t const cacheperdec = std::max(minmem,numblocks ? ((memory + numblocks-1)/numblocks) : 0);

				// offset array
				std::vector<uint64_t> O(rinfo.size(),0);
				for ( uint64_t i = 1; i < rinfo.size(); ++i )
					O[i] = O[i-1] + rinfo[i-1].indexoffset.size();

				libmaus2::autoarray::AutoArray<libmaus2::aio::InputStream::unique_ptr_type> Pindexstreams(rinfo.size());
				libmaus2::autoarray::AutoArray<libmaus2::aio::InputStream::unique_ptr_type> Pdatastreams(rinfo.size());
				libmaus2::autoarray::AutoArray<short_index_decoder_pointer_type> Pdecoders(numblocks);

				// open index and data streams
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < rinfo.size(); ++i )
				{
					libmaus2::aio::InputStream::unique_ptr_type tptr(libmaus2::aio::InputStreamFactoryContainer::constructUnique(rinfo[i].indexfilename));
					Pindexstreams[i] = UNIQUE_PTR_MOVE(tptr);
					libmaus2::aio::InputStream::unique_ptr_type iptr(libmaus2::aio::InputStreamFactoryContainer::constructUnique(rinfo[i].datafilename));
					Pdatastreams[i] = UNIQUE_PTR_MOVE(iptr);
				}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "file open: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				// open indexes
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < rinfo.size(); ++i )
				{
					for ( uint64_t j = 0; j < rinfo[i].indexoffset.size(); ++j )
					{
						Pindexstreams[i]->clear();
						Pindexstreams[i]->seekg(rinfo[i].indexoffset[j]);
						short_index_decoder_pointer_type tptr(new short_index_decoder_type(*Pindexstreams[i],cacheperdec));
						Pdecoders[O[i]+j] = UNIQUE_PTR_MOVE(tptr);
					}
				}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "index setup: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				// compute total number of blocks
				uint64_t const totalbaseblocks = getTotalShortBlocks(Pdecoders);
				#if defined(READENDSBLOCKDEBUG)
				std::cerr << "total number of base blocks is " << totalbaseblocks << std::endl;
				#endif

				// compute split point values
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				// cache only accessor
				ExternalMemoryIndexDecoderShortCountAccessor accessor(Pdecoders);
				std::vector< ::libmaus2::bambam::ReadEnds::hash_value_type > splitPoints(numthreads);
				libmaus2::bambam::ReadEndsBaseShortHashAttributeComparator comp;
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < numthreads; ++i )
				{
					short_index_iterator ita = indexShortBegin(accessor);
					short_index_iterator ite = indexShortEnd(accessor);
					short_index_iterator itc = std::lower_bound(ita,ite,(i * totalbaseblocks)/numthreads );
					splitPoints[i] = itc.i;

					#if 0
					libmaus2::bambam::ReadEnds H;
					H.decodeShortHash(splitPoints[i]);
					std::cerr
						<< "split point "
						<< i << "/" << split
						<< ": ";
					std::cerr << H;
					std::cerr << std::endl;
					#endif
				}

				for ( uint64_t i = 1; i < splitPoints.size(); ++i )
					assert ( splitPoints[i-1] <= splitPoints[i] );

				libmaus2::autoarray::AutoArray< UVal > RR(numthreads * numblocks);
				std::fill(RR.begin(),RR.end(),UVal(std::numeric_limits<uint64_t>::max()));

				// compute split points
				std::vector<
					std::vector< std::pair<uint64_t,uint64_t> >
				> R(numthreads,std::vector<std::pair<uint64_t,uint64_t> >(numblocks));
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				// file on disk
				for ( uint64_t f = 0; f < rinfo.size(); ++f )
					// block of file on disk
					for ( uint64_t k = 0; k < rinfo[f].indexoffset.size(); ++k )
					{
						// part of block of file on disk
						for ( uint64_t i = 0; i < numthreads; ++i )
						{
							libmaus2::bambam::ReadEnds H;
							H.decodeShortHash(splitPoints[i]);

							libmaus2::index::ExternalMemoryIndexDecoderFindLargestSmallerResult<libmaus2::bambam::ReadEndsBase> const
								FLS = Pdecoders[O[f] + k]->findLargestSmaller(H,false /* cache only */);

							uint64_t o = FLS.blockid << libmaus2::bambam::ReadEndsContainerBase::baseIndexShift;
							Pdatastreams[f]->clear();
							Pdatastreams[f]->seekg(FLS.P.first);
							libmaus2::lz::SnappyInputStream SIS(*(Pdatastreams[f]));
							SIS.ignore(FLS.P.second);
							libmaus2::bambam::ReadEnds T;
							uint64_t const blockelcnt = rinfo[f].blockelcnt[k];

							bool first = true;
							libmaus2::bambam::ReadEndsBase prev;

							while ( o < blockelcnt )
							{
								assert ( SIS.peek() >= 0 );
								// std::cerr << ".";
								T.get(SIS);

								if ( first )
								{
									assert ( T == FLS.D );
									first = false;
								}
								else
								{
									assert( prev < T );
								}

								// std::cerr << "H=" << H << " T=" << T << std::endl;
								// break if T is not smaller than H
								if ( ! comp(T,H) )
									break;
								else
									++o;

								prev = T;
							}
							// R.at(i).at(O[f] + k).first = o;

							assert ( RR [ i * numblocks + O[f] + k ].v == std::numeric_limits<uint64_t>::max() );
							RR [ i * numblocks + O[f] + k ].v = o;
						}
					}

				for ( uint64_t i = 0; i < numthreads; ++i )
					for ( uint64_t j = 0; j < numblocks ; ++j )
						R.at(i).at(j).first = RR[i*numblocks+j].v;

				#if 0
				{
					bool gok = true;
					for ( uint64_t j = 0; j < numblocks ; ++j )
						for ( uint64_t i = 1; i < numthreads; ++i )
						{
							bool const ok = R[i-1][j].first <= R[i][j].first;
							if ( ! ok )
								gok = false;
						}
					if ( ! gok )
					{
						for ( uint64_t j = 0; j < numblocks ; ++j )
						{
							std::cerr << "block " << j << std::endl;
							for ( uint64_t i = 0; i < numthreads; ++i )
							{
								bool const lok =
									(i == 0) || (R[i-1][j] <= R[i][j]);
								std::cerr << "\tthread " << i << " " << R[i][j].first << " " << (lok?"":"broken") << std::endl;
							}
						}
						assert ( gok );
					}
				}
				#endif

				// set upper bounds
				for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					for ( uint64_t j = 0; j < R[i].size(); ++j )
						R[i][j].second = R[i+1][j].first;

				if ( numthreads )
				{
					uint64_t j = 0;
					for ( uint64_t k = 0; k < rinfo.size(); ++k )
					{
						ReadEndsBlockDecoderBaseCollectionInfo const & subinfo = rinfo[k];

						for ( uint64_t i = 0; i < subinfo.blockelcnt.size(); ++i )
							R[numthreads-1][j++].second = subinfo.blockelcnt[i];
					}
				}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "split point search: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				#if defined(READENDSBLOCKDEBUG)
				{
					#if defined(READENDSBLOCKTIMING)
					decodersetuprtc.start();
					#endif
					this_type col(rinfo);
					#if defined(READENDSBLOCKTIMING)
					std::cerr << "setup for ReadEndsBlockDecoderBaseCollection " << decodersetuprtc.getElapsedSeconds() << std::endl;
					#endif

					#if defined(READENDSBLOCKTIMING)
					decodersetuprtc.start();
					#endif
					std::vector < std::vector< std::pair<uint64_t,uint64_t> > > VR(numthreads,std::vector< std::pair<uint64_t,uint64_t> >(0));
					for ( uint64_t i = 0; i < numthreads; ++i )
					{
						::libmaus2::bambam::ReadEnds RA;
						RA.decodeShortHash(splitPoints[i]);

						for ( uint64_t j = 0; j < col.size(); ++j )
							VR[i].push_back(std::pair<uint64_t,uint64_t>(col.getBlock(j)->countSmallerThanShort(RA),0));
					}

					for ( uint64_t i = 0; (i+1) < numthreads; ++i )
						for ( uint64_t j = 0; j < VR[i].size(); ++j )
							VR[i][j].second = VR[i+1][j].first;

					if ( numthreads )
					{
						uint64_t j = 0;
						for ( uint64_t k = 0; k < rinfo.size(); ++k )
						{
							ReadEndsBlockDecoderBaseCollectionInfo const & subinfo = rinfo[k];

							for ( uint64_t i = 0; i < subinfo.blockelcnt.size(); ++i )
								VR[numthreads-1][j++].second = subinfo.blockelcnt[i];
						}
					}

					#if defined(READENDSBLOCKTIMING)
					std::cerr << "searching for exact split points " << decodersetuprtc.getElapsedSeconds() << std::endl;
					#endif

					for ( uint64_t i = 0; i < numthreads; ++i )
						for ( uint64_t j = 0; j < VR[i].size(); ++j )
						{
							if ( VR[i][j] != R.at(i).at(j) )
								std::cerr << "i=" << i << " j=" << j << " VR[i][j]=" << VR[i][j].first << "\tR[i][j]=" << R.at(i).at(j).first << std::endl;
							// assert ( VR[i][j].first == R[i][j] );
						}
				}
				#endif

				if ( check )
				{
					std::cerr << "checking...";
					for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					{
						this_type col(rinfo,false /* par */,1 /* numthreads */);

						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[i];
						::libmaus2::bambam::ReadEnds::hash_value_type const splithigh = splitPoints[i+1];

						for ( uint64_t j = 0; j < col.size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[i][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( col.getBlock(j)->get(k).encodeShortHash() >= splitlow );
								assert ( col.getBlock(j)->get(k).encodeShortHash() < splithigh );
							}
						}
					}

					if ( numthreads )
					{
						this_type col(rinfo,false /* par */,1 /* numthreads */);

						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[numthreads-1];

						for ( uint64_t j = 0; j < col.size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[numthreads-1][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( col.getBlock(j)->get(k).encodeShortHash() >= splitlow );
							}
						}
					}
					std::cerr << "done." << std::endl;
				}

				#if 0
				bool gok = true;
				// counter over merge packages (threads)
				for ( uint64_t i = 0; i < R.size(); ++i )
					// counter over blocks
					for ( uint64_t j = 0; j < R[i].size(); ++j )
					{
						std::pair<uint64_t,uint64_t> P = R[i][j];
						bool const ok = P.first <= P.second;
						if ( ! ok )
						{
							std::cerr << std::string(80,'*') << std::endl;
							std::cerr << "late R[" << i << "][" << j << "]=(" << P.first << "," << P.second << ")" << std::endl;

							if ( i > 0 )
								std::cerr << "R[" << i-1 << "][" << j << "]=(" << R[i-1][j].first << "," << R[i-1][j].second << ")" << std::endl;
							if ( i+1 < R.size() )
								std::cerr << "R[" << i+1 << "][" << j << "]=(" << R[i+1][j].first << "," << R[i+1][j].second << ")" << std::endl;

							std::cerr << "late R.size()=" << R.size() << " R[" << i << "].size()=" << R[i].size() << std::endl;
							gok = false;
						}
					}

				assert ( gok );
				#endif

				#if 0
				if ( ! gok )
				{
					std::cerr << std::string(80,'+') << std::endl;

					if ( ! gok )
					{
						for ( uint64_t j = 0; j < numblocks ; ++j )
						{
							std::cerr << "block " << j << std::endl;
							for ( uint64_t i = 0; i < numthreads; ++i )
							{
								bool const lok =
									(i == 0) || (R[i-1][j] <= R[i][j]);
								std::cerr << "\tthread " << i << " " << R[i][j].first << " " << (lok?"":"broken") << std::endl;
							}
						}
						assert ( gok );
					}
				}

				assert ( gok );
				#endif

				return R;
			}

			typedef libmaus2::index::ExternalMemoryIndexDecoder<
				libmaus2::bambam::ReadEndsBase,
				libmaus2::bambam::ReadEndsContainerBase::baseIndexShift,
				libmaus2::bambam::ReadEndsContainerBase::innerIndexShift,
				libmaus2::bambam::ReadEndsBaseLongHashAttributeComparator
			> long_index_decoder_type;
			typedef long_index_decoder_type::unique_ptr_type long_index_decoder_pointer_type;

			static uint64_t getTotalLongBlocks(libmaus2::autoarray::AutoArray<long_index_decoder_pointer_type> const & A)
			{
				uint64_t s = 0;
				for ( uint64_t i = 0; i < A.size(); ++i )
					s += A[i]->size();
				return s;
			}

			struct ExternalMemoryIndexDecoderLongCountAccessor
			{
				typedef ExternalMemoryIndexDecoderLongCountAccessor this_type;
				typedef typename libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
				typedef typename libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;

				libmaus2::autoarray::AutoArray<long_index_decoder_pointer_type> & Pdecoders;

				ExternalMemoryIndexDecoderLongCountAccessor(
					libmaus2::autoarray::AutoArray<long_index_decoder_pointer_type> & Rdecoders
				) : Pdecoders(Rdecoders) {}

				/**
				 * get maximum block start element
				 **/
				libmaus2::bambam::ReadEndsBase max()
				{
					libmaus2::bambam::ReadEndsBase R;
					R.reset();

					for ( uint64_t i = 0; i < Pdecoders.size(); ++i )
						if ( Pdecoders[i]->maxelvalid && R.encodeLongHash() < Pdecoders[i]->maxel.encodeLongHash() )
						{
							R = Pdecoders[i]->maxel;
							R.decodeLongHash(R.encodeLongHash());
						}

					return R;
				}

				uint64_t get(libmaus2::bambam::ReadEndsBase::hash_value_type const & H) const
				{
					libmaus2::bambam::ReadEndsBase R;
					R.decodeLongHash(H);

					uint64_t c = 0;
					for ( uint64_t i = 0; i < Pdecoders.size(); ++i )
						c += Pdecoders[i]->findLargestSmaller(R,true /* cache only */).blockid;

					// std::cerr << "access " << H << " c=" << c << std::endl;

					return c;
				}
			};

			typedef libmaus2::util::UnsignedIntegerIndexIterator<ExternalMemoryIndexDecoderLongCountAccessor,uint64_t,libmaus2::bambam::ReadEndsBase::hash_value_words>
				long_index_iterator;

			static long_index_iterator indexLongBegin(ExternalMemoryIndexDecoderLongCountAccessor & raccessor)
			{
				return long_index_iterator(&raccessor,hashLowerBound());
			}

			static long_index_iterator indexLongEnd  (ExternalMemoryIndexDecoderLongCountAccessor & raccessor)
			{
				return long_index_iterator(&raccessor,raccessor.max().encodeLongHash()+libmaus2::bambam::ReadEndsBase::hash_value_type(1));
			}

			static std::vector < std::vector< std::pair<uint64_t,uint64_t> > > getLongMergeIntervals(
				std::vector<ReadEndsBlockDecoderBaseCollectionInfoBase> const & rinfo,
				uint64_t const numthreads,
				bool const check = false
			)
			{
				#if defined(READENDSBLOCKTIMING)
				libmaus2::timing::RealTimeClock decodersetuprtc;
				#endif
				uint64_t const memory = 256ull*1024ull*1024ull;
				uint64_t const numblocks = computeNumBlocks(rinfo);
				uint64_t const minmem = 16*1024;
				uint64_t const cacheperdec = std::max(minmem,numblocks ? ((memory + numblocks-1)/numblocks) : 0);

				// offset array
				std::vector<uint64_t> O(rinfo.size(),0);
				for ( uint64_t i = 1; i < rinfo.size(); ++i )
					O[i] = O[i-1] + rinfo[i-1].indexoffset.size();

				libmaus2::autoarray::AutoArray<libmaus2::aio::InputStream::unique_ptr_type> Pindexstreams(rinfo.size());
				libmaus2::autoarray::AutoArray<libmaus2::aio::InputStream::unique_ptr_type> Pdatastreams(rinfo.size());
				libmaus2::autoarray::AutoArray<long_index_decoder_pointer_type> Pdecoders(numblocks);

				// open index and data streams
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < rinfo.size(); ++i )
				{
					libmaus2::aio::InputStream::unique_ptr_type tptr(libmaus2::aio::InputStreamFactoryContainer::constructUnique(rinfo[i].indexfilename));
					Pindexstreams[i] = UNIQUE_PTR_MOVE(tptr);
					libmaus2::aio::InputStream::unique_ptr_type iptr(libmaus2::aio::InputStreamFactoryContainer::constructUnique(rinfo[i].datafilename));
					Pdatastreams[i] = UNIQUE_PTR_MOVE(iptr);
				}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "file open: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				// open indexes
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < rinfo.size(); ++i )
					for ( uint64_t j = 0; j < rinfo[i].indexoffset.size(); ++j )
					{
						Pindexstreams[i]->clear();
						Pindexstreams[i]->seekg(rinfo[i].indexoffset[j]);
						long_index_decoder_pointer_type tptr(new long_index_decoder_type(*Pindexstreams[i],cacheperdec));
						Pdecoders[O[i]+j] = UNIQUE_PTR_MOVE(tptr);
					}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "index setup: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				// compute total number of blocks
				uint64_t const totalbaseblocks = getTotalLongBlocks(Pdecoders);
				#if defined(READENDSBLOCKDEBUG)
				std::cerr << "total number of base blocks is " << totalbaseblocks << std::endl;
				#endif

				// compute split point values
				#if defined(READENDSBLOCKTIMING)
				decodersetuprtc.start();
				#endif
				ExternalMemoryIndexDecoderLongCountAccessor accessor(Pdecoders);
				std::vector< ::libmaus2::bambam::ReadEnds::hash_value_type > splitPoints(numthreads);
				libmaus2::bambam::ReadEndsBaseLongHashAttributeComparator comp;
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t i = 0; i < numthreads; ++i )
				{
					long_index_iterator ita = indexLongBegin(accessor);
					long_index_iterator ite = indexLongEnd(accessor);
					long_index_iterator itc = std::lower_bound(ita,ite,(i * totalbaseblocks)/numthreads );
					splitPoints[i] = itc.i;

					#if 0
					libmaus2::bambam::ReadEnds H;
					H.decodeLongHash(splitPoints[i]);
					std::cerr
						<< "split point "
						<< i << "/" << split
						<< ": ";
					std::cerr << H;
					std::cerr << std::endl;
					#endif
				}

				for ( uint64_t i = 1; i < splitPoints.size(); ++i )
					assert ( splitPoints[i-1] <= splitPoints[i] );

				libmaus2::autoarray::AutoArray< UVal > RR(numthreads * numblocks);
				std::fill(RR.begin(),RR.end(),UVal(std::numeric_limits<uint64_t>::max()));

				// compute split points
				std::vector<
					std::vector< std::pair<uint64_t,uint64_t> >
				> R(numthreads,std::vector<std::pair<uint64_t,uint64_t> >(numblocks));
				#if defined(_OPENMP)
				#pragma omp parallel for num_threads(numthreads)
				#endif
				for ( uint64_t f = 0; f < rinfo.size(); ++f )
					for ( uint64_t k = 0; k < rinfo[f].indexoffset.size(); ++k )
						for ( uint64_t i = 0; i < numthreads; ++i )
						{
							libmaus2::bambam::ReadEnds H;
							H.decodeLongHash(splitPoints[i]);

							libmaus2::index::ExternalMemoryIndexDecoderFindLargestSmallerResult<libmaus2::bambam::ReadEndsBase> const
								FLS = Pdecoders[O[f] + k]->findLargestSmaller(H,false /* cache only */);

							uint64_t o = FLS.blockid << libmaus2::bambam::ReadEndsContainerBase::baseIndexShift;
							Pdatastreams[f]->clear();
							Pdatastreams[f]->seekg(FLS.P.first);
							libmaus2::lz::SnappyInputStream SIS(*(Pdatastreams[f]));
							SIS.ignore(FLS.P.second);
							libmaus2::bambam::ReadEnds T;
							uint64_t const blockelcnt = rinfo[f].blockelcnt[k];

							while ( o < blockelcnt )
							{
								assert ( SIS.peek() >= 0 );
								T.get(SIS);
								// break if T is not smaller than H
								if ( ! comp(T,H) )
									break;
								else
									++o;
							}
							assert ( RR [ i * numblocks + O[f] + k ].v == std::numeric_limits<uint64_t>::max() );
							RR [ i * numblocks + O[f] + k ].v = o;
						}

				for ( uint64_t i = 0; i < numthreads; ++i )
					for ( uint64_t j = 0; j < numblocks ; ++j )
						R.at(i).at(j).first = RR[i*numblocks+j].v;

				#if 0
				{
					bool gok = true;
					for ( uint64_t j = 0; j < numblocks ; ++j )
						for ( uint64_t i = 1; i < numthreads; ++i )
						{
							bool const ok = R[i-1][j].first <= R[i][j].first;
							if ( ! ok )
								gok = false;
						}
					if ( ! gok )
					{
						for ( uint64_t j = 0; j < numblocks ; ++j )
						{
							std::cerr << "block " << j << std::endl;
							for ( uint64_t i = 0; i < numthreads; ++i )
							{
								bool const lok =
									(i == 0) || (R[i-1][j] <= R[i][j]);
								std::cerr << "\tthread " << i << " " << R[i][j].first << " " << (lok?"":"broken") << std::endl;
							}
						}
						assert ( gok );
					}
				}
				#endif

				// set upper bounds
				for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					for ( uint64_t j = 0; j < R[i].size(); ++j )
						R[i][j].second = R[i+1][j].first;

				if ( numthreads )
				{
					uint64_t j = 0;
					for ( uint64_t k = 0; k < rinfo.size(); ++k )
					{
						ReadEndsBlockDecoderBaseCollectionInfo const & subinfo = rinfo[k];

						for ( uint64_t i = 0; i < subinfo.blockelcnt.size(); ++i )
							R[numthreads-1][j++].second = subinfo.blockelcnt[i];
					}
				}
				#if defined(READENDSBLOCKTIMING)
				std::cerr << "split point search: " << decodersetuprtc.getElapsedSeconds() << std::endl;
				#endif

				#if defined(READENDSBLOCKDEBUG)
				{
					#if defined(READENDSBLOCKTIMING)
					decodersetuprtc.start();
					#endif
					this_type col(rinfo);
					#if defined(READENDSBLOCKTIMING)
					std::cerr << "setup for ReadEndsBlockDecoderBaseCollection " << decodersetuprtc.getElapsedSeconds() << std::endl;
					#endif

					#if defined(READENDSBLOCKTIMING)
					decodersetuprtc.start();
					#endif
					std::vector < std::vector< std::pair<uint64_t,uint64_t> > > VR(numthreads,std::vector< std::pair<uint64_t,uint64_t> >(0));
					for ( uint64_t i = 0; i < numthreads; ++i )
					{
						::libmaus2::bambam::ReadEnds RA;
						RA.decodeLongHash(splitPoints[i]);

						for ( uint64_t j = 0; j < col.size(); ++j )
							VR[i].push_back(std::pair<uint64_t,uint64_t>(col.getBlock(j)->countSmallerThanLong(RA),0));
					}

					for ( uint64_t i = 0; (i+1) < numthreads; ++i )
						for ( uint64_t j = 0; j < VR[i].size(); ++j )
							VR[i][j].second = VR[i+1][j].first;

					if ( numthreads )
					{
						uint64_t j = 0;
						for ( uint64_t k = 0; k < rinfo.size(); ++k )
						{
							ReadEndsBlockDecoderBaseCollectionInfo const & subinfo = rinfo[k];

							for ( uint64_t i = 0; i < subinfo.blockelcnt.size(); ++i )
								VR[numthreads-1][j++].second = subinfo.blockelcnt[i];
						}
					}

					#if defined(READENDSBLOCKTIMING)
					std::cerr << "searching for exact split points " << decodersetuprtc.getElapsedSeconds() << std::endl;
					#endif

					for ( uint64_t i = 0; i < numthreads; ++i )
						for ( uint64_t j = 0; j < VR[i].size(); ++j )
						{
							if ( VR[i][j] != R.at(i).at(j) )
								std::cerr << "i=" << i << " j=" << j << " VR[i][j]=" << VR[i][j].first << "\tR[i][j]=" << R.at(i).at(j).first << std::endl;
							// assert ( VR[i][j].first == R[i][j] );
						}
				}
				#endif

				if ( check )
				{
					std::cerr << "checking...";
					for ( uint64_t i = 0; (i+1) < numthreads; ++i )
					{
						this_type col(rinfo,false /* par */, 1 /* numthreads */);

						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[i];
						::libmaus2::bambam::ReadEnds::hash_value_type const splithigh = splitPoints[i+1];

						for ( uint64_t j = 0; j < col.size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[i][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( col.getBlock(j)->get(k).encodeLongHash() >= splitlow );
								assert ( col.getBlock(j)->get(k).encodeLongHash() < splithigh );
							}
						}
					}

					if ( numthreads )
					{
						this_type col(rinfo,false /* par */, 1 /* numthreads */);

						::libmaus2::bambam::ReadEnds::hash_value_type const splitlow = splitPoints[numthreads-1];

						for ( uint64_t j = 0; j < col.size(); ++j )
						{
							std::pair<uint64_t,uint64_t> ind = R[numthreads-1][j];

							for ( uint64_t k = ind.first; k < ind.second; ++k )
							{
								assert ( col.getBlock(j)->get(k).encodeLongHash() >= splitlow );
							}
						}
					}
					std::cerr << "done." << std::endl;
				}

				#if 0
				for ( uint64_t i = 0; i < R.size(); ++i )
					for ( uint64_t j = 0; j < R[i].size(); ++j )
					{
						std::pair<uint64_t,uint64_t> P = R[i][j];
						bool const ok = P.first <= P.second;
						if ( ! ok )
						{
							std::cerr << "R[" << i << "][" << j << "]=(" << P.first << "," << P.second << ")" << std::endl;
							assert ( ok );
						}
					}
				#endif

				return R;
			}

			void merge(
				std::vector< std::pair<uint64_t,uint64_t> > V,
				std::ostream & out, std::iostream & indexout
			) const
			{
				libmaus2::index::ExternalMemoryIndexGenerator<
					libmaus2::bambam::ReadEndsBase,
					libmaus2::bambam::ReadEndsContainerBase::baseIndexShift,
					libmaus2::bambam::ReadEndsContainerBase::innerIndexShift
				> generator(indexout);

				/* uint64_t indexpos = */ generator.setup();

				//! pair of list index and ReadEnds object
				typedef std::pair<uint64_t,::libmaus2::bambam::ReadEnds> qtype;
				//! merge heap
				std::priority_queue<qtype,std::vector<qtype>,::libmaus2::bambam::ReadEndsHeapPairComparator> Q;

				for ( uint64_t i = 0; i < V.size(); ++i )
					if ( V[i].first < V[i].second )
						Q.push(
							qtype(
								i,
								getBlock(i)->get(V[i].first++)
							)
						);

				uint64_t ind = 0;
				libmaus2::lz::SnappyOutputStream<std::ostream> SOS(out);
				while ( Q.size() )
				{
					qtype const P = Q.top();
					Q.pop();

					P.second.put(SOS);
					if ( ((ind++) & ReadEndsContainerBase::baseIndexMask) == 0 )
						generator.put(P.second,SOS.getOffset());

					if ( V[P.first].first < V[P.first].second )
						Q.push(
							qtype(
								P.first,
								getBlock(P.first)->get(V[P.first].first++)
							)
						);
				}

				SOS.flush();
				generator.flush();

				out.flush();
				indexout.flush();
			}
		};
	}
}
#endif
