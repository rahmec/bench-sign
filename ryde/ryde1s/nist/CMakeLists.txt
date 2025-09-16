enable_language(C)
get_property(scheme_targets DIRECTORY ${CMAKE_SOURCE_DIR}/src PROPERTY BUILDSYSTEM_TARGETS)

message(STATUS "${SCHEME} NIST API")

set(KAT_SRC PQCgenKAT_sign.c sign.c)
set(KAT_HDR api.h)

set(lib ${PROJECT_NAME}-${PARAMETER})
set(KAT_APP kat-${lib})
string(REPLACE - "" INSTANCE ${lib})
string(TOUPPER "${INSTANCE}" PARAMETER_SET)

message(STATUS "Creating test target: ${KAT_APP}")

add_executable(${KAT_APP} ${KAT_SRC} ${KAT_HDR})
target_link_libraries(${KAT_APP} LINK_PUBLIC ${lib})
target_compile_options(${KAT_APP} PUBLIC -D_${PARAMETER_SET}_)
target_include_directories(${KAT_APP} PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/library ${CMAKE_SOURCE_DIR}/src/library/rng ${CMAKE_SOURCE_DIR}/src/tcith ${CMAKE_SOURCE_DIR}/src/${SRCA})
add_dependencies(${KAT_APP} ${lib})
target_compile_options(${KAT_APP} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${KAT_APP} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
