enable_language(C)
message(STATUS "${SCHEME} Basefield Library")

# Create a library target for the given parameter defined in parent DEGREE variable
message(STATUS "Creating library target ${PROJECT_NAME}-field-${DEGREE}")
add_library(${PROJECT_NAME}-field-${DEGREE} STATIC field_${DEGREE}.c)
target_include_directories(${PROJECT_NAME}-field-${DEGREE} PRIVATE ../${SRCA} ..)
target_compile_options(${PROJECT_NAME}-field-${DEGREE} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")
target_compile_options(${PROJECT_NAME}-field-${DEGREE} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
