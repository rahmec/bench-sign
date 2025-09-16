enable_language(C)

message(STATUS "NIST Random Number Generator (RNG) - Required for NIST KATs")

find_package(OpenSSL REQUIRED)

if (OPENSSL_FOUND)
    message(STATUS "OpenSSL: Version ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL: include dir at ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL: libraries at ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES}")
endif()

add_library(rng OBJECT rng.c)
target_link_libraries(rng PUBLIC OpenSSL::Crypto)
