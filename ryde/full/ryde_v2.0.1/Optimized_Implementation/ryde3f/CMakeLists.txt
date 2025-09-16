cmake_minimum_required(VERSION 3.22)
project(ryde C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE OFF)
enable_testing()

string(TOUPPER "${PROJECT_NAME}" SCHEME)

# Needed for nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Define default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif()

set(ALLOWED_WARNINGS " -Wall -Wextra -Wno-unused-function")
set(NEEDED_OPT_FLAGS " -mpclmul -msse2 -mavx -mavx2 -maes")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG -O0 -gdwarf-5 -Wall ${ALLOWED_WARNINGS} ${NEEDED_OPT_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -flto -Wall -ftree-vectorize -funroll-loops ${ALLOWED_WARNINGS} ${NEEDED_OPT_FLAGS}")

set(ASM_OPTIONS "-x assembler-with-cpp -Wa,-defsym,old_gas_syntax=1 -Wa,-defsym,no_plt=1")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

add_compile_definitions(
    "$<$<BOOL:${BOOLTEST}>:BOOLTEST>"
    "$<$<STREQUAL:${BENCHMARKING},CYCLES>:CYCLES>"
    "$<$<STREQUAL:${BENCHMARKING},TIME>:TIME>"
    "$<$<STREQUAL:${ARCHITECTURE},ARMV7l>:ARMV7l>"
    "$<$<STREQUAL:${ARCHITECTURE},AARCH64>:AARCH64>"
    "$<$<STREQUAL:${ARCHITECTURE},x8664>:x8664>"
)

set(PARAMETER 3f)
set(BITS 384)
set(LAMBDA 192)
set(DEGREE 61)

add_subdirectory(src)
add_subdirectory(nist)
add_subdirectory(bench)
add_subdirectory(example)
