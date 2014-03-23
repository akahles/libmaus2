/*
    libmaus
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
#if ! defined(LIBMAUS_PARALLEL_POSIXSPINLOCK_HPP)
#define LIBMAUS_PARALLEL_POSIXSPINLOCK_HPP

#include <libmaus/LibMausConfig.hpp>
#include <libmaus/exception/LibMausException.hpp>
#include <libmaus/util/unique_ptr.hpp>
#include <libmaus/util/shared_ptr.hpp>
#include <cerrno>

#if defined(LIBMAUS_HAVE_PTHREADS)
#include <pthread.h>

namespace libmaus
{
	namespace parallel
	{
                struct PosixSpinLock
                {
                	typedef PosixSpinLock this_type;
                	typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
                	typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
                
                        pthread_spinlock_t spinlock;
                        
                        PosixSpinLock() : spinlock()
                        {
                                pthread_spin_init(&spinlock,0);
                        }
                        ~PosixSpinLock()
                        {
                                pthread_spin_destroy(&spinlock);
                        }
                        
                        void lock()
                        {
                        	if ( pthread_spin_lock ( &spinlock ) )
                        	{
                        		::libmaus::exception::LibMausException se;
                        		se.getStream() << "pthread_spin_lock failed" << std::endl;
                        		se.finish();
                        		throw se;
                        	}
                        }
                        void unlock()
                        {
                        	if ( pthread_spin_unlock ( &spinlock ) )
                        	{
                        		::libmaus::exception::LibMausException se;
                        		se.getStream() << "pthread_spin_unlock failed" << std::endl;
                        		se.finish();
                        		throw se;                        	
                        	}
                        }
                        /**
                         * try to lock spin lock. returns true if locking was succesful, false if lock
                         * was already locked
                         **/
                        bool trylock()
                        {
                        	int const r = pthread_spin_trylock(&spinlock);
                        	if ( r == EBUSY )
                        	{
                        		return false;
				}
				else if ( r )
				{
                        		::libmaus::exception::LibMausException se;
                        		se.getStream() << "pthread_spin_trylock failed" << std::endl;
                        		se.finish();
                        		throw se;                        						
				}
				else
				{
					return true;
				}
                        }
                        
                        /*
                         * try to lock spin lock. if succesful, lock is unlocked and return value is true,
                         * otherwise return value is false
                         */
                        bool tryLockUnlock()
                        {
                        	bool const r = trylock();
                        	if ( r )
                        		unlock();
				return r;
                        }
                };
                
                struct ScopePosixSpinLock
                {
                	PosixSpinLock & spinlock;
                	
                	ScopePosixSpinLock(PosixSpinLock & rspinlock)
                	: spinlock(rspinlock)
                	{
                		spinlock.lock();
                	}
                	~ScopePosixSpinLock()
                	{
                		spinlock.unlock();
                	}
                };
	}
}
#endif
#endif
