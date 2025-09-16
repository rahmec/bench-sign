enable_language(C)
get_property(scheme_targets DIRECTORY ${CMAKE_SOURCE_DIR}/src PROPERTY BUILDSYSTEM_TARGETS)

message(STATUS "${SCHEME} Benchmark")

set(BENCH_SRC bench.c ../nist/sign.c)
set(BENCH_HDR ../nist/api.h)

set(lib ${PROJECT_NAME}-${PARAMETER})
set(BENCH_APP bench-${lib})
string(REPLACE - "" INSTANCE ${lib})
string(TOUPPER "${INSTANCE}" PARAMETER_SET)

message(STATUS "Creating test target: ${BENCH_APP}")

add_executable(${BENCH_APP} ${BENCH_SRC} ${BENCH_HDR})
target_link_libraries(${BENCH_APP} LINK_PUBLIC ${lib})
target_compile_options(${BENCH_APP} PUBLIC -D_${PARAMETER_SET}_)
target_include_directories(${BENCH_APP} PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/library ${CMAKE_SOURCE_DIR}/src/library/rng ${CMAKE_SOURCE_DIR}/src/tcith ${CMAKE_SOURCE_DIR}/src/${SRCA} ${CMAKE_SOURCE_DIR}/nist)
add_dependencies(${BENCH_APP} ${lib})
target_compile_options(${BENCH_APP} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${BENCH_APP} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
