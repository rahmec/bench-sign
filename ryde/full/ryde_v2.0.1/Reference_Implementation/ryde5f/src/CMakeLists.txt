enable_language(C)

set(SRC
    ggm_tree.c
    field_vector.c
    field_matrix.c
    binary_matrix.c
    parsing.c
    keygen.c
    tcith.c
    tcith_sign.c
    tcith_verify.c
)

set(FSHAKE)
set(SRCA)
if(OPT_IMPL STREQUAL "avx")
    message(STATUS "Optimized Implementation (avx)")
    set(SRCA optimized/avx/)
    set(FSHAKE -D_SHAKE_TIMES4_)
else()
    message(STATUS "Reference Implementation (ref)")
    set(SRCA reference/)
endif()
set(FSHA)
if(CMT_IMPL STREQUAL "SHA3")
    message(STATUS "Commit calculations by employing SHA3")
    set(FSHA -D_SHA3_)
else()
    message(STATUS "Commit calculations by employing Rijndael")
endif()
set(VERBOSE_MODE -DVERBOSE)

add_subdirectory(library)
add_subdirectory(field)

message(STATUS "${SCHEME} Library")

message(STATUS "Creating library target ${CMAKE_PROJECT_NAME}-${PARAMETER}")

set(SRCP parameters/${PROJECT_NAME}${PARAMETER})

set(HDR rijndael/seed_expand_rijndael_${LAMBDA}.h ryde.h)

# Create a library target for the given parameter defined in parent BITS variable
add_library(${PROJECT_NAME}-${PARAMETER} STATIC ${HDR} ${SRC})
target_include_directories(${PROJECT_NAME}-${PARAMETER} PRIVATE ${SCRP} ${SRCA} library library/rng rijndael)
target_link_libraries(${PROJECT_NAME}-${PARAMETER} ${PROJECT_NAME}-hash-${BITS} ${PROJECT_NAME}-hash-x4-${BITS} ${PROJECT_NAME}-field-${DEGREE} rng)
target_compile_options(${PROJECT_NAME}-${PARAMETER} PUBLIC ${FSHA} ${FSHAKE})
target_compile_options(${PROJECT_NAME}-${PARAMETER} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}-${PARAMETER} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")

add_library(${PROJECT_NAME}-${PARAMETER}-verbose STATIC ${HDR} ${SRC})
target_include_directories(${PROJECT_NAME}-${PARAMETER}-verbose PRIVATE ${SCRP} ${SRCA} library library/rng rijndael)
target_link_libraries(${PROJECT_NAME}-${PARAMETER}-verbose ${PROJECT_NAME}-hash-${BITS} ${PROJECT_NAME}-hash-x4-${BITS} ${PROJECT_NAME}-field-${DEGREE} rng)
target_compile_options(${PROJECT_NAME}-${PARAMETER}-verbose PUBLIC ${FSHA} ${FSHAKE} ${VERBOSE_MODE})
target_compile_options(${PROJECT_NAME}-${PARAMETER}-verbose PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}-${PARAMETER}-verbose PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
