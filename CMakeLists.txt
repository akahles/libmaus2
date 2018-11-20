cmake_minimum_required (VERSION 3.6.1)
project(Libmaus2)
include(CheckIncludeFileCXX)
include(CheckIncludeFile)
include(FindOpenMP)
include(FindThreads)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)
include(CheckStructHasMember)
include(CheckTypeSize)
include(CheckCXXSourceRuns)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER "")

## SET PRECOMPILER DIRECTIVES
TEST_BIG_ENDIAN(BIGENDIAN)
if(BIGENDIAN)
  string(APPEND HEADER "#define LIBMAUS2_BYTE_ORDER_BIG_ENDIAN\n")
else()
  string(APPEND HEADER "#define LIBMAUS2_BYTE_ORDER_LITTLE_ENDIAN\n")
endif()

CHECK_INCLUDE_FILE_CXX("cstdint" HAS_CSTDINT)

CHECK_INCLUDE_FILE("windows.h" HAS_WINDOWS_H)
CHECK_INCLUDE_FILE("sys/time.h" HAS_SYS_TIME_H)
CHECK_INCLUDE_FILE("sys/types.h" HAS_SYS_TYPES_H)
CHECK_INCLUDE_FILE("stdint.h" HAS_STDINT_H)
CHECK_INCLUDE_FILE("sys/vfs.h" HAS_SYSVFS_H)
CHECK_INCLUDE_FILE("aio.h" HAS_AIO_H)
CHECK_INCLUDE_FILE("unistd.h" HAS_UNISTD_H)
CHECK_INCLUDE_FILE("fcntl.h" HAS_FCNTL_H)
CHECK_INCLUDE_FILE("execinfo.h" HAS_EXECINFO_H)
CHECK_INCLUDE_FILE("sys/prctl.h" HAS_PRCTL_H)
CHECK_INCLUDE_FILE("features.h" HAS_FEATURES_H)
CHECK_INCLUDE_FILE("semaphore.h" HAS_SEMAPHORE_H)
CHECK_INCLUDE_FILE("libkern/OSAtomic.h" HAS_OSATOMIC_H)
CHECK_INCLUDE_FILE("sys/mman.h" HAS_MMAP_H)
CHECK_INCLUDE_FILE("smmintrin.h" HAS_SMMINTRIN_H)
CHECK_INCLUDE_FILE("archive.h" HAS_LIBARCHIVE_H)

if(HAS_CSTDINT)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_CSTDINT\n")
endif()
if(HAS_WINDOWS_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_WINDOWS_H\n")
endif()
if(HAS_SYS_TIME_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SYS_TIME_H\n")
endif()
if(HAS_SYS_TYPES_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SYS_TYPES_H\n")
endif()
if(HAS_SYSVFS_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SYS_VFS_H\n")
  check_symbol_exists(statfs "sys/vfs.h" STATFS)
  if(STATFS)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_STATFS\n")
    check_struct_has_member("struct statfs" f_bsize sys/vfs.h F_BSIZE)
    if(F_BSIZE)
      string(APPEND HEADER "#define LIBMAUS2_HAVE_STATFS_F_BSIZE\n")
    endif()
    check_struct_has_member("struct statfs" f_iosize sys/vfs.h F_IOSIZE)
    if(F_IOSIZE)
      string(APPEND HEADER "#define LIBMAUS2_HAVE_STATFS_F_IOSIZE\n")
    endif()
  endif()
endif()
if(HAS_AIO_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_AIO_H\n")
endif()
if(HAS_UNISTD_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_UNISTD_H\n")
  check_function_exists(getpagesize GETPAGESIZE)
  if(GETPAGESIZE)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_GETPAGESIZE\n")
  endif()
endif()
if(HAS_FCNTL_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_FCNTL\n")
endif()
if(HAS_EXECINFO_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_EXECINFO_H\n")
  check_function_exists(backtrace BACKTRACE)
  if(BACKTRACE)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_BACKTRACE\n")
  endif()
endif()
if(HAS_PRCTL_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_PRCTL\n")
endif()
if(HAS_FEATURES_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_FEATURES_H\n")
endif()
if(HAS_BACKTRACE)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_BACKTRACE\n")
endif()
if(HAS_SEMAPHORE_H)
  set(CMAKE_REQUIRED_FLAGS " -pthread")
  check_symbol_exists("sem_init" "semaphore.h" SEMINIT)
  if(SEMINIT)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_SEM_INIT\n")
  endif()

  check_symbol_exists("sem_destroy" "semaphore.h" SEMDESTROY)
  if(SEMDESTROY)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_SEM_DESTROY\n")
  endif()

  check_symbol_exists("sem_wait" "semaphore.h" SEMWAIT)
  if(SEMWAIT)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_SEM_WAIT\n")
  endif()

  check_symbol_exists("sem_timedwait" "semaphore.h" SEMTIMEDWAIT)
  if(SEMTIMEDWAIT)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_SEM_TIMEDWAIT\n")
  endif()

  check_symbol_exists("sem_post" "semaphore.h" SEMPOST)
  if(SEMPOST)
    string(APPEND HEADER "#define HAVE_SEM_POST\n")
  endif()
endif()
if(HAS_OSATOMIC_H)
  check_function_exists(OSSpinLockLock DARWINSPINLOCKS)
  if(DARWINSPINLOCKS)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_DARWIN_SPINLOCKS\n")
  endif()
endif()
if(HAS_MMAP_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_MMAP\n")
  check_symbol_exists(MAP_HUGETLB "sys/mman.h" MMAP_HUGEPAGES)
  if(MMAP_HUGEPAGES)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_MMAP_HUGEPAGES\n")
  endif()
endif()
if(HAS_SMMINTRIN_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SMMINTRIN_H\n")
endif()
if(HAS_LIBARCHIVE_H)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_LIBARCHIVE\n")
endif()

check_function_exists(posix_memalign POSIXMEMALIGN)
if(POSIXMEMALIGN)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_POSIX_MEMALIGN\n")
endif()

check_function_exists(setrlimit SETRLIMIT)
if(SETRLIMIT)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SETRLIMIT\n")
endif()

if(CMAKE_USE_PTHREADS_INIT)
  set(CMAKE_REQUIRED_FLAGS " -pthread")
  string(APPEND CMAKE_CXX_FLAGS " -pthread")
  string(APPEND HEADER "#define LIBMAUS2_HAVE_PTHREADS\n")

  check_function_exists(pthread_setaffinity_np CHECKSETAFFINITY)
  if(CHECKSETAFFINITY)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_PTHREAD_SETAFFINITY_NP\n")
  endif()

  check_type_size("pthread_spinlock_t" SIZEOF_SPINLOCK)
  if(NOT SIZEOF_SPINLOCK STREQUAL "")
    string(APPEND HEADER "#define LIBMAUS2_HAVE_POSIX_SPINLOCKS\n")
  else()
    message(STATUS "POSIX spinlocks not found")
  endif()

  check_function_exists(pthread_setname_np PTHREAD_SETNAME_NP)
  if(PTHREAD_SETNAME_NP)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_HAVEPTHREADSETNAME_NP\n")
  endif()
  check_function_exists(pthread_set_name_np PTHREAD_SET_NAME_NP)
  if(PTHREAD_SET_NAME_NP)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_HAVEPTHREADSET_NAME_NP\n")
  endif()
  check_function_exists(pthread_getname_np PTHREAD_GETNAME_NP)
  if(PTHREAD_GETNAME_NP)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_HAVEPTHREADGETNAME_NP\n")
  endif()
  check_function_exists(pthread_get_name_np PTHREAD_GET_NAME_NP)
  if(PTHREAD_GET_NAME_NP)
    string(APPEND HEADER "#define LIBMAUS2_HAVE_HAVEPTHREADGET_NAME_NP\n")
  endif()
endif()

check_cxx_source_runs("
int
main ()
{
unsigned long long v = 0; __sync_fetch_and_add(&v,1)
  ;
  return 0;
}" SYNCOPS)
if(SYNCOPS)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SYNC_OPS\n")
endif()

check_cxx_source_runs("
int
main ()
{
unsigned int v = 0; __sync_lock_test_and_set(&v,1)
  ;
  return 0;
}
" SYNCLOCK)
if(SYNCLOCK)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SYNC_LOCK\n")
endif()

## from https://gist.github.com/UnaNancyOwen/263c243ae1e05a2f9d0e
set(CMAKE_REQUIRED_FLAGS "-mavx")
check_cxx_source_runs("
        #include <immintrin.h>
        int main()
        {
          __m256 a, b, c;
          const float src[8] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
          float dst[8];
          a = _mm256_loadu_ps( src );
          b = _mm256_loadu_ps( src );
          c = _mm256_add_ps( a, b );
          _mm256_storeu_ps( dst, c );
          for( int i = 0; i < 8; i++ ){
            if( ( src[i] + src[i] ) != dst[i] ){
              return -1;
            }
          }
          return 0;
        }"
        HAVE_AVX_EXTENSIONS)
if(HAVE_AVX_EXTENSIONS)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_AVX\n")
  string(APPEND CMAKE_CXX_FLAGS " -mavx")
endif()

set(CMAKE_REQUIRED_FLAGS "-mavx2")
check_cxx_source_runs("
        #include <immintrin.h>
        int main()
        {
          __m256i a, b, c;
          const int src[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
          int dst[8];
          a =  _mm256_loadu_si256( (__m256i*)src );
          b =  _mm256_loadu_si256( (__m256i*)src );
          c = _mm256_add_epi32( a, b );
          _mm256_storeu_si256( (__m256i*)dst, c );
          for( int i = 0; i < 8; i++ ){
            if( ( src[i] + src[i] ) != dst[i] ){
              return -1;
            }
          }
          return 0;
        }"
        HAVE_AVX2_EXTENSIONS)
if(HAVE_AVX2_EXTENSIONS)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_AVX2\n")
  string(APPEND CMAKE_CXX_FLAGS " -mavx2")
endif()

set(CMAKE_REQUIRED_FLAGS "-msse4.2")
check_cxx_source_runs("int main() { ; return 0; }" HAVE_SSE4)
if(HAVE_SSE4)
  string(APPEND HEADER "#define HAVE_SSE4\n")
  string(APPEND CMAKE_CXX_FLAGS " -msse4.2")
endif()

set(CMAKE_REQUIRED_FLAGS "-mssse3")
check_cxx_source_runs("int main() { ; return 0; }" HAVE_SSSE3)
if(HAVE_SSSE3)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SSSE3\n")
  string(APPEND CMAKE_CXX_FLAGS " -mssse3")
endif()

check_cxx_source_runs("
#include <sys/types.h>
 /* Check that off_t can represent 2**63 - 1 correctly.
    We can't simply define LARGE_OFF_T to be 9223372036854775807,
    since some C++ compilers masquerading as C compilers
    incorrectly reject 9223372036854775807.  */
#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
  int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721
               && LARGE_OFF_T % 2147483647 == 1)
              ? 1 : -1];
int
main ()
{

  ;
  return 0;
}
" FILEOFFSET_BITS)
if(FILEOFFSET_BITS)
  string(APPEND HEADER "#define _FILE_OFFSET_BITS 64\n")
endif()

check_symbol_exists(MSG_CMSG_CLOEXEC "sys/types.h;sys/socket.h" MSG_CMSG_CLOEXEC)
if(MSG_CMSG_CLOEXEC)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_MSG_CMSG_CLOEXEC\n")
endif()

if(WITH_ASSEMBLY)
  message(STATUS "Enabling assembly")
  string(APPEND HEADER "#define LIBMAUS2_USE_ASSEMBLY\n")
endif()
if(WITH_YASM)
  message(STATUS "Enabling SHA2 assembly")
  string(APPEND HEADER "#define LIBMAUS2_HAVE_SHA2_ASSEMBLY\n")
endif()

if(WITH_GLUTLS)
  message(STATUS "Enabling GLUTLS")
  string(APPEND HEADER "#define LIBMAUS2_HAVE_GNUTLS\n")
endif()

string(APPEND HEADER "#define LIBMAUS2_HAVE_i386\n")
cmake_host_system_information(RESULT HAS_X8664 QUERY IS_64BIT)
if(HAS_X8664)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_x86_64\n")
endif()

check_type_size("unsigned __int128" SIZEOF_UINT128)
if(SIZEOF_UINT128 EQUAL 16)
  string(APPEND HEADER "#define LIBMAUS2_HAVE_UNSIGNED_INT128\n")
endif()

check_type_size("unsigned long" SIZEOF_ULONG)
string(APPEND HEADER "#define LIBMAUS2_SIZEOF_UNSIGNED_LONG ${SIZEOF_ULONG}\n")

string(APPEND CMAKE_CXX_FLAGS " \
  -Wall -Wextra \
  -D_THREAD_SAFE \
") # -DDBGDEBUG -Wconversion -Wsign-conversion -Werror=shadow=compatible-local

if(WITH_DEATH_TESTS)
message(STATUS "Enabling death tests in gtest")
string(APPEND CMAKE_CXX_FLAGS " -D_DEATH_TEST")
endif()

if(WITH_AUTOARRAYTRACE)
message(STATUS "Enabling autoarraytrace")
string(APPEND HEADER "#define LIBMAUS2_AUTOARRAY_AUTOARRAYTRACE 7\n")
endif()


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RELEASE")
endif()

find_package(OpenMP REQUIRED)
if(NOT TARGET OpenMP::OpenMP_CXX)
    add_library(OpenMP_TARGET INTERFACE)
    add_library(OpenMP::OpenMP_CXX ALIAS OpenMP_TARGET)
    target_compile_options(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
    find_package(Threads REQUIRED)
    target_link_libraries(OpenMP_TARGET INTERFACE Threads::Threads)
    target_link_libraries(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
  string(APPEND HEADER "#define LIBMAUS2_DEBUG_COMPILED\n")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/libmaus2)
file(WRITE ${CMAKE_BINARY_DIR}/include/libmaus2/LibMausConfig.hpp "${HEADER}\n")


# Profile build type
set(CMAKE_CXX_FLAGS_PROFILE "-pg -DNDEBUG -O2 -g")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg -g")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "-pg -g")

# Debug build type
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")

set(CMAKE_VERBOSE_MAKEFILE 1)
cmake_policy(SET CMP0015 NEW)

include_directories(
    "src"
    ${CMAKE_BINARY_DIR}/include
)

link_directories(
)

file(GLOB_RECURSE src_files
  "src/libmaus2/*/*.cpp"
)

add_library(maus2 STATIC ${src_files})

set(METALIBS ${METALIBS}
)

if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -static)
endif()

target_link_libraries(
  maus2 ${METALIBS}
)

install(TARGETS maus2 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY src/libmaus2 DESTINATION ${CMAKE_INSTALL_PREFIX}/include PATTERN src/libmaus2/*/*.cpp EXCLUDE)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/libmaus2 DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

## tests
file( GLOB TEST_SOURCES src/test/*.cpp )
link_directories(
)

# adapted from https://stackoverflow.com/questions/14306642/adding-multiple-executables-in-cmake
foreach( testsourcefile ${TEST_SOURCES} )
    get_filename_component(EXNAME ${testsourcefile} NAME_WE)
    add_executable( ${EXNAME} ${testsourcefile} )
    target_link_libraries( ${EXNAME} maus2 ${METALIBS} )
endforeach( testsourcefile ${TEST_SOURCES} )
