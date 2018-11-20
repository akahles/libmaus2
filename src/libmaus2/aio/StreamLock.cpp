#include <libmaus2/aio/StreamLock.hpp>

namespace libmaus2
{
    namespace aio
    {
        libmaus2::parallel::PosixSpinLock StreamLock::coutlock;
        libmaus2::parallel::PosixSpinLock StreamLock::cerrlock;
        libmaus2::parallel::PosixSpinLock StreamLock::cinlock;
    }
}
