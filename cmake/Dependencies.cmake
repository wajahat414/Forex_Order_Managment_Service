# Dependencies.cmake - Centralized dependency management for OMS

# =================================================================== 
# External Dependencies
# ===================================================================

# Dependencies - Order matters! OpenSSL must be found before FastDDS
find_package(OpenSSL REQUIRED)

# Create OpenSSL targets if they don't exist (compatibility with older CMake)
if(OpenSSL_FOUND AND NOT TARGET OpenSSL::SSL)
    add_library(OpenSSL::SSL UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
endif()

if(OpenSSL_FOUND AND NOT TARGET OpenSSL::Crypto)
    add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
endif()

# FastDDS and related
find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)

# JSON library
find_package(nlohmann_json REQUIRED)

# Optional: REST API support
# find_package(Drogon REQUIRED) # Enable if you add REST

# =================================================================== 
# Local Dependencies
# ===================================================================

# log4cxx (local installation)
set(LOG4CXX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/include")
set(LOG4CXX_LIBRARY_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/lib")
set(LOG4CXX_LIBRARY      "${LOG4CXX_LIBRARY_DIR}/liblog4cxx.dylib")

if(NOT EXISTS ${LOG4CXX_LIBRARY})
  message(FATAL_ERROR "log4cxx library not found at ${LOG4CXX_LIBRARY}")
endif()

# =================================================================== 
# Dependency Information
# ===================================================================

message(STATUS "Dependencies configured:")
message(STATUS "  - OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "  - FastDDS: Found")
message(STATUS "  - nlohmann_json: Found") 
message(STATUS "  - log4cxx: ${LOG4CXX_LIBRARY}")
