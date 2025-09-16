enable_language(C)
get_property(scheme_targets DIRECTORY ${CMAKE_SOURCE_DIR}/src PROPERTY BUILDSYSTEM_TARGETS)

message(STATUS "${SCHEME} Example")

set(EXAMPLE_SRC example.c ../nist/sign.c)
set(EXAMPLE_HDR ../nist/api.h)

set(lib ${PROJECT_NAME}-${PARAMETER})
set(EXAMPLE_APP example-${lib})
string(REPLACE - "" INSTANCE ${lib})
string(TOUPPER "${INSTANCE}" PARAMETER_SET)

message(STATUS "Creating test target: ${EXAMPLE_APP}")

add_executable(${EXAMPLE_APP} ${EXAMPLE_SRC} ${EXAMPLE_HDR})
target_link_libraries(${EXAMPLE_APP} LINK_PUBLIC ${lib}-verbose)
target_compile_options(${EXAMPLE_APP} PUBLIC -D_${PARAMETER_SET}_ ${VERBOSE_MODE})
target_include_directories(${EXAMPLE_APP} PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/library ${CMAKE_SOURCE_DIR}/src/library/rng ${CMAKE_SOURCE_DIR}/src/tcith ${CMAKE_SOURCE_DIR}/src/${SRCA} ${CMAKE_SOURCE_DIR}/nist)
add_dependencies(${EXAMPLE_APP} ${lib}-verbose)
target_compile_options(${EXAMPLE_APP} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${EXAMPLE_APP} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
