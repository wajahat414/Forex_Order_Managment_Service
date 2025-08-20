cmake_minimum_required(VERSION 3.20)

# ===================================================================
# Project Configuration
# ===================================================================

project(OMS 
  VERSION 1.0.0
  DESCRIPTION "Order Management Service for Forex Trading"
  LANGUAGES CXX
)

# ===================================================================
# Build Configuration
# ===================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Local FastDDS install prefix (dds/) first in search path
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dds;${CMAKE_PREFIX_PATH}")

# Enable threading
find_package(Threads REQUIRED)

# ===================================================================
# Include Modular CMake Files
# ===================================================================

include(cmake/Dependencies.cmake)
include(cmake/Sources.cmake)
include(cmake/Tests.cmake)
include(cmake/Utilities.cmake)

# ===================================================================
# Main OMS Executable
# ===================================================================

add_executable(${PROJECT_NAME} 
  ${CORE_SOURCES}
  ${IDL_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR}/idl/idl
  ${CMAKE_CURRENT_SOURCE_DIR}/dds/include
  ${LOG4CXX_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  fastdds
  fastcdr
  nlohmann_json::nlohmann_json
  ${LOG4CXX_LIBRARY}
  Threads::Threads
)

# Library search paths
target_link_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dds/lib
  ${LOG4CXX_LIBRARY_DIR}
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
  $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# ===================================================================
# Platform-Specific Configuration
# ===================================================================

if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path/../lib;@loader_path/../lib;${CMAKE_CURRENT_SOURCE_DIR}/dds/lib;${LOG4CXX_LIBRARY_DIR}"
    MACOSX_RPATH ON
  )
endif()

# ===================================================================
# Installation
# ===================================================================

install(TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION bin
)

install(DIRECTORY config/
  DESTINATION etc/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.json"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/log4cxx.xml")
  install(FILES config/log4cxx.xml 
    DESTINATION etc/${PROJECT_NAME}
  )
endif()

# ===================================================================
# Build Summary
# ===================================================================

message(STATUS "")
message(STATUS "=== OMS Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
