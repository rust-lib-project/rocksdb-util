# Prerequisites for Windows:
#     This cmake build is for Windows 64-bit only.
#
# Prerequisites:
#     You must have at least Visual Studio 2019. Start the Developer Command Prompt window that is a part of Visual Studio installation.
#     Run the build commands from within the Developer Command Prompt window to have paths to the compiler and runtime libraries set.
#     You must have git.exe in your %PATH% environment variable.
#
# To build Rocksdb for Windows is as easy as 1-2-3-4-5:
#
# 1. Update paths to third-party libraries in thirdparty.inc file
# 2. Create a new directory for build artifacts
#        mkdir build
#        cd build
# 3. Run cmake to generate project files for Windows, add more options to enable required third-party libraries.
#    See thirdparty.inc for more information.
#        sample command: cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DWITH_GFLAGS=1 -DWITH_SNAPPY=1 -DWITH_JEMALLOC=1 -DWITH_JNI=1 ..
# 4. Then build the project in debug mode (you may want to add /m[:<N>] flag to run msbuild in <N> parallel threads
#                                          or simply /m to use all avail cores)
#        msbuild rocksdb.sln
#
#        rocksdb.sln build features exclusions of test only code in Release. If you build ALL_BUILD then everything
#        will be attempted but test only code does not build in Release mode.
#
# 5. And release mode (/m[:<N>] is also supported)
#        msbuild rocksdb.sln /p:Configuration=Release
#
# Linux:
#
# 1. Install a recent toolchain if you're on a older distro. C++17 required (GCC >= 7, Clang >= 5)
# 2. mkdir build; cd build
# 3. cmake ..
# 4. make -j

cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")
include(ReadVersion)
include(GoogleTest)
get_rocksdb_version(rocksdb_VERSION)
project(rocksdb-util
        VERSION ${rocksdb_VERSION}
        LANGUAGES CXX C ASM)

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(default_build_type "Debug")
    else()
        set(default_build_type "RelWithDebInfo")
    endif()
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
            "Default BUILD_TYPE is ${default_build_type}" FORCE)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

if ($ENV{CIRCLECI})
    message(STATUS "Build for CircieCI env, a few tests may be disabled")
    add_definitions(-DCIRCLECI)
endif()

if( NOT DEFINED CMAKE_CXX_STANDARD )
    set(CMAKE_CXX_STANDARD 17)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /nologo /EHsc /GS /Gd /GR /GF /fp:precise /Zc:wchar_t /Zc:forScope /errorReport:queue")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC /d2Zi+ /W4 /wd4127 /wd4800 /wd4996 /wd4351 /wd4100 /wd4204 /wd4324")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wextra -Wall -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-compare -Wshadow -Wno-unused-parameter -Wno-unused-variable -Woverloaded-virtual -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-strict-aliasing -Wno-invalid-offsetof")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
    endif()
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format")
        add_definitions(-D_POSIX_C_SOURCE=1)
    endif()
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("-momit-leaf-frame-pointer" HAVE_OMIT_LEAF_FRAME_POINTER)
        if(HAVE_OMIT_LEAF_FRAME_POINTER)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -momit-leaf-frame-pointer")
        endif()
    endif()
endif()

include(CheckCCompilerFlag)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")
    CHECK_C_COMPILER_FLAG("-mcpu=power9" HAS_POWER9)
    if(HAS_POWER9)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=power9 -mtune=power9")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=power9 -mtune=power9")
    else()
        CHECK_C_COMPILER_FLAG("-mcpu=power8" HAS_POWER8)
        if(HAS_POWER8)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=power8 -mtune=power8")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=power8 -mtune=power8")
        endif(HAS_POWER8)
    endif(HAS_POWER9)
    CHECK_C_COMPILER_FLAG("-maltivec" HAS_ALTIVEC)
    if(HAS_ALTIVEC)
        message(STATUS " HAS_ALTIVEC yes")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maltivec")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maltivec")
    endif(HAS_ALTIVEC)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|AARCH64")
    CHECK_C_COMPILER_FLAG("-march=armv8-a+crc+crypto" HAS_ARMV8_CRC)
    if(HAS_ARMV8_CRC)
        message(STATUS " HAS_ARMV8_CRC yes")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc+crypto -Wno-unused-function")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc+crypto -Wno-unused-function")
    endif(HAS_ARMV8_CRC)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|AARCH64")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "s390x")
    CHECK_C_COMPILER_FLAG("-march=native" HAS_S390X_MARCH_NATIVE)
    if(HAS_S390X_MARCH_NATIVE)
        message(STATUS " HAS_S390X_MARCH_NATIVE yes")
    endif(HAS_S390X_MARCH_NATIVE)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "s390x")


include(CheckCXXSourceCompiles)
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
if(NOT MSVC)
    set(CMAKE_REQUIRED_FLAGS "-msse4.2 -mpclmul")
endif()

if (NOT PORTABLE OR FORCE_SSE42)
    CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <nmmintrin.h>
#include <wmmintrin.h>
int main() {
  volatile uint32_t x = _mm_crc32_u32(0, 0);
  const auto a = _mm_set_epi64x(0, 0);
  const auto b = _mm_set_epi64x(0, 0);
  const auto c = _mm_clmulepi64_si128(a, b, 0x00);
  auto d = _mm_cvtsi128_si64(c);
}
" HAVE_SSE42)
    if(HAVE_SSE42)
        add_definitions(-DHAVE_SSE42)
        add_definitions(-DHAVE_PCLMUL)
    elseif(FORCE_SSE42)
        message(FATAL_ERROR "FORCE_SSE42=ON but unable to compile with SSE4.2 enabled")
    endif()
endif()

# Check if -latomic is required or not
if (NOT MSVC)
    set(CMAKE_REQUIRED_FLAGS "--std=c++17")
    CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
std::atomic<uint64_t> x(0);
int main() {
  uint64_t i = x.load(std::memory_order_relaxed);
  bool b = x.is_lock_free();
  return 0;
}
" BUILTIN_ATOMIC)
    if (NOT BUILTIN_ATOMIC)
        #TODO: Check if -latomic exists
        list(APPEND THIRDPARTY_LIBS atomic)
    endif()
endif()

if (WITH_LIBURING)
    find_package(uring)
    if (uring_FOUND)
        add_definitions(-DROCKSDB_IOURING_PRESENT)
        list(APPEND THIRDPARTY_LIBS uring::uring)
    endif()
endif()

# Reset the required flags
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})

option(WITH_IOSTATS_CONTEXT "Enable IO stats context" ON)
if (NOT WITH_IOSTATS_CONTEXT)
    add_definitions(-DNIOSTATS_CONTEXT)
endif()

option(WITH_PERF_CONTEXT "Enable perf context" ON)
if (NOT WITH_PERF_CONTEXT)
    add_definitions(-DNPERF_CONTEXT)
endif()

option(FAIL_ON_WARNINGS "Treat compile warnings as errors" ON)
if(FAIL_ON_WARNINGS)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    else() # assume GCC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif()
endif()

option(WITH_ASAN "build with ASAN" OFF)
if(WITH_ASAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    if(WITH_JEMALLOC)
        message(FATAL "ASAN does not work well with JeMalloc")
    endif()
endif()

option(WITH_TSAN "build with TSAN" OFF)
if(WITH_TSAN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -Wl,-pie")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -fPIC")
    if(WITH_JEMALLOC)
        message(FATAL "TSAN does not work well with JeMalloc")
    endif()
endif()

option(WITH_UBSAN "build with UBSAN" OFF)
if(WITH_UBSAN)
    add_definitions(-DROCKSDB_UBSAN_RUN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    if(WITH_JEMALLOC)
        message(FATAL "UBSAN does not work well with JeMalloc")
    endif()
endif()

option(WITH_NUMA "build with NUMA policy support" OFF)
if(WITH_NUMA)
    find_package(NUMA REQUIRED)
    add_definitions(-DNUMA)
    include_directories(${NUMA_INCLUDE_DIR})
    list(APPEND THIRDPARTY_LIBS NUMA::NUMA)
endif()

option(WITH_TBB "build with Threading Building Blocks (TBB)" OFF)
if(WITH_TBB)
    find_package(TBB REQUIRED)
    add_definitions(-DTBB)
    list(APPEND THIRDPARTY_LIBS TBB::TBB)
endif()

# Stall notifications eat some performance from inserts
option(DISABLE_STALL_NOTIF "Build with stall notifications" OFF)
if(DISABLE_STALL_NOTIF)
    add_definitions(-DROCKSDB_DISABLE_STALL_NOTIFICATION)
endif()

option(WITH_DYNAMIC_EXTENSION "build with dynamic extension support" OFF)
if(NOT WITH_DYNAMIC_EXTENSION)
    add_definitions(-DROCKSDB_NO_DYNAMIC_EXTENSION)
endif()

option(ASSERT_STATUS_CHECKED "build with assert status checked" OFF)
if (ASSERT_STATUS_CHECKED)
    message(STATUS "Build with assert status checked")
    add_definitions(-DROCKSDB_ASSERT_STATUS_CHECKED)
endif()


# RTTI is by default AUTO which enables it in debug and disables it in release.
set(USE_RTTI AUTO CACHE STRING "Enable RTTI in builds")
set_property(CACHE USE_RTTI PROPERTY STRINGS AUTO ON OFF)
if(USE_RTTI STREQUAL "AUTO")
    message(STATUS "Enabling RTTI in Debug builds only (default)")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DROCKSDB_USE_RTTI")
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")
    endif()
elseif(USE_RTTI)
    message(STATUS "Enabling RTTI in all builds")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DROCKSDB_USE_RTTI")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DROCKSDB_USE_RTTI")
else()
    if(MSVC)
        message(STATUS "Disabling RTTI in Release builds. Always on in Debug.")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DROCKSDB_USE_RTTI")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
    else()
        message(STATUS "Disabling RTTI in all builds")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")
    endif()
endif()

# Used to run CI build and tests so we can run faster
option(OPTDBG "Build optimized debug build with MSVC" OFF)
option(WITH_RUNTIME_DEBUG "build with debug version of runtime library" ON)
if(MSVC)
    if(OPTDBG)
        message(STATUS "Debug optimization is enabled")
        set(CMAKE_CXX_FLAGS_DEBUG "/Oxt")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /RTC1")

        # Minimal Build is deprecated after MSVC 2015
        if( MSVC_VERSION GREATER 1900 )
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Gm-")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Gm")
        endif()

    endif()
    if(WITH_RUNTIME_DEBUG)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /${RUNTIME_LIBRARY}d")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /${RUNTIME_LIBRARY}")
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oxt /Zp8 /Gm- /Gy /${RUNTIME_LIBRARY}")

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-memcmp")
endif()

option(ROCKSDB_LITE "Build RocksDBLite version" OFF)
if(ROCKSDB_LITE)
    add_definitions(-DROCKSDB_LITE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -Os")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Cygwin")
    add_definitions(-fno-builtin-memcmp -DCYGWIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_definitions(-DOS_MACOSX)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-DOS_LINUX)
elseif(CMAKE_SYSTEM_NAME MATCHES "SunOS")
    add_definitions(-DOS_SOLARIS)
elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD")
    add_definitions(-DOS_GNU_KFREEBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    add_definitions(-DOS_FREEBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    add_definitions(-DOS_NETBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    add_definitions(-DOS_OPENBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly")
    add_definitions(-DOS_DRAGONFLYBSD)
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
    add_definitions(-DOS_ANDROID)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions(-DWIN32 -DOS_WIN -D_MBCS -DWIN64 -DNOMINMAX)
    if(MINGW)
        add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_VISTA)
    endif()
endif()

if(NOT WIN32)
    add_definitions(-DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX)
endif()

option(WITH_FALLOCATE "build with fallocate" ON)
if(WITH_FALLOCATE)
    CHECK_CXX_SOURCE_COMPILES("
#include <fcntl.h>
#include <linux/falloc.h>
int main() {
 int fd = open(\"/dev/null\", 0);
 fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, 1024);
}
" HAVE_FALLOCATE)
    if(HAVE_FALLOCATE)
        add_definitions(-DROCKSDB_FALLOCATE_PRESENT)
    endif()
endif()

CHECK_CXX_SOURCE_COMPILES("
#include <fcntl.h>
int main() {
  int fd = open(\"/dev/null\", 0);
  sync_file_range(fd, 0, 1024, SYNC_FILE_RANGE_WRITE);
}
" HAVE_SYNC_FILE_RANGE_WRITE)
if(HAVE_SYNC_FILE_RANGE_WRITE)
    add_definitions(-DROCKSDB_RANGESYNC_PRESENT)
endif()

CHECK_CXX_SOURCE_COMPILES("
#include <pthread.h>
int main() {
  (void) PTHREAD_MUTEX_ADAPTIVE_NP;
}
" HAVE_PTHREAD_MUTEX_ADAPTIVE_NP)
if(HAVE_PTHREAD_MUTEX_ADAPTIVE_NP)
    add_definitions(-DROCKSDB_PTHREAD_ADAPTIVE_MUTEX)
endif()

include(CheckCXXSymbolExists)
if(CMAKE_SYSTEM_NAME MATCHES "^FreeBSD")
    check_cxx_symbol_exists(malloc_usable_size malloc_np.h HAVE_MALLOC_USABLE_SIZE)
else()
    check_cxx_symbol_exists(malloc_usable_size malloc.h HAVE_MALLOC_USABLE_SIZE)
endif()
if(HAVE_MALLOC_USABLE_SIZE)
    add_definitions(-DROCKSDB_MALLOC_USABLE_SIZE)
endif()

check_cxx_symbol_exists(sched_getcpu sched.h HAVE_SCHED_GETCPU)
if(HAVE_SCHED_GETCPU)
    add_definitions(-DROCKSDB_SCHED_GETCPU_PRESENT)
endif()

check_cxx_symbol_exists(getauxval auvx.h HAVE_AUXV_GETAUXVAL)
if(HAVE_AUXV_GETAUXVAL)
    add_definitions(-DROCKSDB_AUXV_GETAUXVAL_PRESENT)
endif()

check_cxx_symbol_exists(F_FULLFSYNC "fcntl.h" HAVE_FULLFSYNC)
if(HAVE_FULLFSYNC)
    add_definitions(-DHAVE_FULLFSYNC)
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
if(USE_FOLLY)
    include_directories(${PROJECT_SOURCE_DIR}/third-party/folly)
    add_definitions(-DUSE_FOLLY -DFOLLY_NO_CONFIG)
    list(APPEND THIRDPARTY_LIBS glog)
endif()
find_package(Threads REQUIRED)

# Main library source code

set(SOURCES
        c.cc
        util/async_file_reader.cc
        util/cleanable.cc
        util/coding.cc
        util/crc32c.cc
        util/hash.cc
        util/murmurhash.cc
        util/random.cc
        util/ribbon_config.cc
        util/slice.cc
        util/filter_policy.cc
        util/status.cc
        util/string_util.cc
        util/thread_local.cc
        util/xxhash.cc
        )

if(WIN32)
    list(APPEND SOURCES
            port/win/io_win.cc
            port/win/env_win.cc
            port/win/env_default.cc
            port/win/port_win.cc
            port/win/win_logger.cc
            port/win/win_thread.cc)
    if(WITH_XPRESS)
        list(APPEND SOURCES
                port/win/xpress_win.cc)
    endif()

    if(WITH_JEMALLOC)
        list(APPEND SOURCES
                port/win/win_jemalloc.cc)
    endif()

else()
    list(APPEND SOURCES port/port_posix.cc)
endif()

set(ROCKSDB_STATIC_LIB rocksdb-util${ARTIFACT_SUFFIX})
set(ROCKSDB_SHARED_LIB rocksdb-util-shared${ARTIFACT_SUFFIX})

if(HAVE_SSE42 AND NOT MSVC)
    set_source_files_properties(
            util/crc32c.cc
            PROPERTIES COMPILE_FLAGS "-msse4.2 -mpclmul")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")
    list(APPEND SOURCES
            util/crc32c_ppc.c
            util/crc32c_ppc_asm.S)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64")

if(HAS_ARMV8_CRC)
    list(APPEND SOURCES
            util/crc32c_arm64.cc)
endif(HAS_ARMV8_CRC)

if(WIN32)
    list(APPEND SOURCES
            port/win/io_win.cc
            port/win/env_win.cc
            port/win/env_default.cc
            port/win/port_win.cc
            port/win/win_logger.cc
            port/win/win_thread.cc)
    if(WITH_XPRESS)
        list(APPEND SOURCES
                port/win/xpress_win.cc)
    endif()

    if(WITH_JEMALLOC)
        list(APPEND SOURCES
                port/win/win_jemalloc.cc)
    endif()

else()
    list(APPEND SOURCES
            port/port_posix.cc)
endif()

option(ROCKSDB_BUILD_SHARED "Build shared versions of the RocksDB libraries" ON)


if(WIN32)
    set(SYSTEM_LIBS ${SYSTEM_LIBS} shlwapi.lib rpcrt4.lib)
else()
    set(SYSTEM_LIBS ${CMAKE_THREAD_LIBS_INIT})
endif()

set(ROCKSDB_PLUGIN_EXTERNS "")
set(ROCKSDB_PLUGIN_BUILTINS "")
message(STATUS "ROCKSDB PLUGINS TO BUILD ${ROCKSDB_PLUGINS}")
list(APPEND PLUGINS ${ROCKSDB_PLUGINS})
foreach(PLUGIN IN LISTS PLUGINS)
    set(PLUGIN_ROOT "${CMAKE_SOURCE_DIR}/plugin/${PLUGIN}/")
    message("including rocksb plugin ${PLUGIN_ROOT}")
    set(PLUGINMKFILE "${PLUGIN_ROOT}${PLUGIN}.mk")
    if (NOT EXISTS ${PLUGINMKFILE})
        message(FATAL_ERROR "Missing plugin makefile: ${PLUGINMKFILE}")
    endif()
    file(READ ${PLUGINMKFILE} PLUGINMK)
    string(REGEX MATCH "SOURCES = ([^\n]*)" FOO ${PLUGINMK})
    set(MK_SOURCES ${CMAKE_MATCH_1})
    separate_arguments(MK_SOURCES)
    foreach(MK_FILE IN LISTS MK_SOURCES)
        list(APPEND SOURCES "${PLUGIN_ROOT}${MK_FILE}")
    endforeach()
    string(REGEX MATCH "_FUNC = ([^\n]*)" FOO ${PLUGINMK})
    if (NOT ${CMAKE_MATCH_1} STREQUAL "")
        string(APPEND ROCKSDB_PLUGIN_BUILTINS "{\"${PLUGIN}\", " ${CMAKE_MATCH_1} "},")
        string(APPEND ROCKSDB_PLUGIN_EXTERNS "int " ${CMAKE_MATCH_1} "(ROCKSDB_NAMESPACE::ObjectLibrary&, const std::string&); ")
    endif()
    string(REGEX MATCH "_LIBS = ([^\n]*)" FOO ${PLUGINMK})
    if (NOT ${CMAKE_MATCH_1} STREQUAL "")
        list(APPEND THIRDPARTY_LIBS "${CMAKE_MATCH_1}")
    endif()
    message("THIRDPARTY_LIBS=${THIRDPARTY_LIBS}")
    #TODO: We need to set any compile/link-time flags and add any link libraries
endforeach()

string(TIMESTAMP TS "%Y-%m-%d %H:%M:%S" UTC)
set(BUILD_DATE "${TS}" CACHE STRING "the time we first built rocksdb")

find_package(Git)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE GIT_SHA COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD )
    execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" RESULT_VARIABLE GIT_MOD COMMAND "${GIT_EXECUTABLE}" diff-index HEAD --quiet)
    execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE GIT_DATE COMMAND "${GIT_EXECUTABLE}" log -1 --date=format:"%Y-%m-%d %T" --format="%ad")
    execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE GIT_TAG RESULT_VARIABLE rv COMMAND "${GIT_EXECUTABLE}" symbolic-ref -q --short HEAD OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (rv AND NOT rv EQUAL 0)
        execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE GIT_TAG COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
else()
    set(GIT_SHA 0)
    set(GIT_MOD 1)
endif()
string(REGEX REPLACE "[^0-9a-fA-F]+" "" GIT_SHA "${GIT_SHA}")
string(REGEX REPLACE "[^0-9: /-]+" "" GIT_DATE "${GIT_DATE}")

set(BUILD_VERSION_CC ${CMAKE_BINARY_DIR}/build_version.cc)
configure_file(util/build_version.cc.in ${BUILD_VERSION_CC} @ONLY)

add_library(${ROCKSDB_STATIC_LIB} STATIC ${SOURCES} ${BUILD_VERSION_CC})
target_link_libraries(${ROCKSDB_STATIC_LIB} PRIVATE
        ${THIRDPARTY_LIBS} ${SYSTEM_LIBS})

if(ROCKSDB_BUILD_SHARED)
    add_library(${ROCKSDB_SHARED_LIB} SHARED ${SOURCES} ${BUILD_VERSION_CC})
    target_link_libraries(${ROCKSDB_SHARED_LIB} PRIVATE
            ${THIRDPARTY_LIBS} ${SYSTEM_LIBS})

    if(WIN32)
        set_target_properties(${ROCKSDB_SHARED_LIB} PROPERTIES
                COMPILE_DEFINITIONS "ROCKSDB_DLL;ROCKSDB_LIBRARY_EXPORTS")
        if(MSVC)
            set_target_properties(${ROCKSDB_STATIC_LIB} PROPERTIES
                    COMPILE_FLAGS "/Fd${CMAKE_CFG_INTDIR}/${ROCKSDB_STATIC_LIB}.pdb")
            set_target_properties(${ROCKSDB_SHARED_LIB} PROPERTIES
                    COMPILE_FLAGS "/Fd${CMAKE_CFG_INTDIR}/${ROCKSDB_SHARED_LIB}.pdb")
        endif()
    else()
        set_target_properties(${ROCKSDB_SHARED_LIB} PROPERTIES
                LINKER_LANGUAGE CXX
                VERSION ${rocksdb_VERSION})
    endif()
endif()

if(ROCKSDB_BUILD_SHARED AND NOT WIN32)
    set(ROCKSDB_LIB ${ROCKSDB_SHARED_LIB})
else()
    set(ROCKSDB_LIB ${ROCKSDB_STATIC_LIB})
endif()

if(NOT WIN32 OR ROCKSDB_INSTALL_ON_WINDOWS)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            # Change default installation prefix on Linux to /usr
            set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
        endif()
    endif()

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(package_config_destination ${CMAKE_INSTALL_LIBDIR}/cmake/rocksdb)

    configure_package_config_file(
            ${CMAKE_CURRENT_LIST_DIR}/cmake/RocksDBConfig.cmake.in RocksDBConfig.cmake
            INSTALL_DESTINATION ${package_config_destination}
    )

    write_basic_package_version_file(
            RocksDBConfigVersion.cmake
            VERSION ${rocksdb_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    configure_file(
            ${PROJECT_NAME}.pc.in
            ${PROJECT_NAME}.pc
            @ONLY
    )

    install(DIRECTORY include/rocksdb COMPONENT devel DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    foreach (plugin ${PLUGINS})
        foreach (header ${${plugin}_HEADERS})
            install(FILES plugin/${plugin}/${header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rocksdb/plugin/${plugin})
        endforeach()
    endforeach()

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/cmake/modules" COMPONENT devel DESTINATION ${package_config_destination})

    install(
            TARGETS ${ROCKSDB_STATIC_LIB}
            EXPORT RocksDBTargets
            COMPONENT devel
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    if(ROCKSDB_BUILD_SHARED)
        install(
                TARGETS ${ROCKSDB_SHARED_LIB}
                EXPORT RocksDBTargets
                COMPONENT runtime
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        )
    endif()

    install(
            EXPORT RocksDBTargets
            COMPONENT devel
            DESTINATION ${package_config_destination}
            NAMESPACE RocksDB::
    )

    install(
            FILES
            ${CMAKE_CURRENT_BINARY_DIR}/RocksDBConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/RocksDBConfigVersion.cmake
            COMPONENT devel
            DESTINATION ${package_config_destination}
    )

    install(
            FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
            COMPONENT devel
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()


option(WITH_EXAMPLES "build with examples" OFF)
if(WITH_EXAMPLES)
    add_subdirectory(examples)
endif()

