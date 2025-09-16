enable_language(C)
message(STATUS "${SCHEME} hash (sha3) Library")

# Build XKCP library
add_subdirectory(xkcp)

# Build rng library (NIST rng concerning NIST KATs)
add_subdirectory(rng)

# Create a library target for the given parameter defined in parent BITS variable
message(STATUS "Creating library target ${PROJECT_NAME}-hash-${BITS}")
add_library(${PROJECT_NAME}-hash-${BITS} STATIC hash_sha3_${BITS}.c)
target_include_directories(${PROJECT_NAME}-hash-${BITS} PRIVATE library/xkcp)
target_link_libraries(${PROJECT_NAME}-hash-${BITS} xkcp)
target_compile_options(${PROJECT_NAME}-hash-${BITS} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}-hash-${BITS} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")

# Create a library target for the given parameter defined in parent BITS variable
message(STATUS "Creating library target ${PROJECT_NAME}-hash-x4-${BITS}")
add_library(${PROJECT_NAME}-hash-x4-${BITS} STATIC hash_sha3_x4_${BITS}.c)
target_include_directories(${PROJECT_NAME}-hash-x4-${BITS} PRIVATE library/xkcp)
target_link_libraries(${PROJECT_NAME}-hash-x4-${BITS} xkcp)
target_compile_options(${PROJECT_NAME}-hash-x4-${BITS} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}-hash-x4-${BITS} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")