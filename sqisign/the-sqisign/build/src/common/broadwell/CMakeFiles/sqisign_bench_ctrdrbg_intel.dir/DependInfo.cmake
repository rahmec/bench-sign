
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/vaes256_key_expansion.S" "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/build/src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/vaes256_key_expansion.S.o"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "CTRDRBG_TEST_BENCH"
  "ENABLE_SIGN"
  "HAVE_UINT128"
  "RADIX_64"
  "RANDOMBYTES_INIT_PLATFORM=randombytes_init_aes_ni"
  "RANDOMBYTES_PLATFORM=randombytes_aes_ni"
  "SQISIGN_BUILD_TYPE_BROADWELL"
  "SQISIGN_GF_IMPL_BROADWELL"
  "SQISIGN_TEST_REPS=10"
  "TARGET_AMD64"
  "TARGET_OS_UNIX"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/include"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/generic/include"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/include"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/../ref/include"
  )

# The set of dependency files which are needed:
set(CMAKE_DEPENDS_DEPENDENCY_FILES
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/generic/randombytes_system.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/generic/randombytes_system.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/generic/randombytes_system.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/generic/test/bench_ctrdrbg.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/generic/test/bench_ctrdrbg.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/generic/test/bench_ctrdrbg.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/ref/aes_c.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/ref/aes_c.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/ref/aes_c.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/ref/randombytes_ctrdrbg.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/ref/randombytes_ctrdrbg.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/__/ref/randombytes_ctrdrbg.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/aes_ni.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/aes_ni.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/aes_ni.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/ctr_drbg.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/ctr_drbg.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/ctr_drbg.c.o.d"
  "/home/raximus/repos/work/speck/implementation/bench/sqisign/the-sqisign/src/common/broadwell/randombytes_ctrdrbg_aesni.c" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/randombytes_ctrdrbg_aesni.c.o" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/randombytes_ctrdrbg_aesni.c.o.d"
  "" "src/common/broadwell/sqisign_bench_ctrdrbg_intel" "gcc" "src/common/broadwell/CMakeFiles/sqisign_bench_ctrdrbg_intel.dir/link.d"
  )

# Targets to which this target links which contain Fortran sources.
set(CMAKE_Fortran_TARGET_LINKED_INFO_FILES
  )

# Targets to which this target links which contain Fortran sources.
set(CMAKE_Fortran_TARGET_FORWARD_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
