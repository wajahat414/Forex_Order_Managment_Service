cmake_minimum_required(VERSION 3.28)

project(oms_modular_monolith LANGUAGES CXX)

# Toolchain / standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optional C++20 module support in CMake
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 1)
# We'll control module scanning manually based on support detection

# Local FastDDS install prefix (dds/) first in search path
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dds;${CMAKE_PREFIX_PATH}")

# Dependencies
find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)
find_package(nlohmann_json REQUIRED)
# find_package(Drogon REQUIRED) # Enable if you add REST

# log4cxx (local)
set(LOG4CXX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/include")
set(LOG4CXX_LIBRARY_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/lib")
set(LOG4CXX_LIBRARY      "${LOG4CXX_LIBRARY_DIR}/liblog4cxx.dylib")

if(NOT EXISTS ${LOG4CXX_LIBRARY})
  message(FATAL_ERROR "log4cxx library not found at ${LOG4CXX_LIBRARY}")
endif()

# Source discovery
file(GLOB_RECURSE CORE_SOURCES
  "src/*.cpp"
  "src/*.cxx"
  "src/*.cc"
)
file(GLOB IDL_SOURCES
  "idl/idl/*PubSubTypes.cxx"
  "idl/idl/*TypeObjectSupport.cxx"
)

# Exclude example client from main
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/test_client\\.cpp$")
# Exclude modules smoke test when modules are disabled
if(NOT (ENABLE_MODULES AND MODULES_SUPPORTED AND OMS_MODULE_FILES))
  list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/modules_smoke_test\\.cpp$")
endif()

# Diagnostics
message(STATUS "Discovered core sources:")
foreach(f ${CORE_SOURCES})
  message(STATUS "  ${f}")
endforeach()
message(STATUS "Discovered IDL sources:")
foreach(f ${IDL_SOURCES})
  message(STATUS "  ${f}")
endforeach()

# Main executable
add_executable(${PROJECT_NAME}
  ${CORE_SOURCES}
  ${IDL_SOURCES}
)

# Target includes (modern, per-target)
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

# macOS runtime paths
if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path/../lib;@loader_path/../lib;${CMAKE_CURRENT_SOURCE_DIR}/dds/lib;${LOG4CXX_LIBRARY_DIR}"
    MACOSX_RPATH ON
  )
endif()

# ============ C++20 modules (optional, guarded) ============
option(ENABLE_MODULES "Build C++20 module library" ON)

# Detect generators and compilers that support modules
set(MODULES_SUPPORTED FALSE)
if(CMAKE_GENERATOR MATCHES "Ninja")
  # Check if we're using a compiler that supports modules well
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0")
    set(MODULES_SUPPORTED TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0")
    set(MODULES_SUPPORTED TRUE)
  endif()
elseif(MSVC AND CMAKE_GENERATOR MATCHES "Visual Studio")
  set(MODULES_SUPPORTED TRUE)
endif()

# Override: Disable modules on macOS with Apple clang for now (experimental support)
if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  set(MODULES_SUPPORTED FALSE)
  message(STATUS "C++20 modules disabled for Apple clang (experimental support)")
endif()

# Collect module files that actually exist (support either SimpleRiskService.ixx or RiskService.ixx)
set(OMS_MODULE_CANDIDATES
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/services/IRiskService.ixx
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/services/SimpleRiskService.ixx
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/services/RiskService.ixx
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/position/PositionService.ixx
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/oms/ExecutionCache.ixx
)

set(OMS_MODULE_FILES)
foreach(m ${OMS_MODULE_CANDIDATES})
  if(EXISTS "${m}")
    list(APPEND OMS_MODULE_FILES "${m}")
  endif()
endforeach()

if(ENABLE_MODULES AND MODULES_SUPPORTED AND OMS_MODULE_FILES)
  # Enable module scanning only when we're actually using modules
  set(CMAKE_CXX_SCAN_FOR_MODULES ON)
  
  add_library(oms_modules)
  target_compile_features(oms_modules PRIVATE cxx_std_20)
  target_sources(oms_modules
    PRIVATE
      FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${OMS_MODULE_FILES}
  )
  target_link_libraries(${PROJECT_NAME} PRIVATE oms_modules)
  message(STATUS "C++20 module library 'oms_modules' enabled (generator: ${CMAKE_GENERATOR})")
else()
  # Explicitly disable module scanning when modules aren't supported
  set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
  message(STATUS "C++20 modules disabled or unsupported by generator '${CMAKE_GENERATOR}'.")
endif()

# Optional: ModulesSmokeTest only when modules are enabled
if(TARGET oms_modules AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/modules_smoke_test.cpp")
  add_executable(ModulesSmokeTest src/modules_smoke_test.cpp)
  target_link_libraries(ModulesSmokeTest PRIVATE oms_modules)
  message(STATUS "ModulesSmokeTest configured")
endif()

# Utilities
add_custom_target(regenerate_idl
  COMMAND echo "Regenerating FastDDS IDL files..."
  COMMAND fastddsgen -replace -d idl OrderMessage.idl
  COMMAND fastddsgen -replace -d idl NewOrderSingle.idl
  COMMAND fastddsgen -replace -d idl ExecutionReport.idl
  COMMAND fastddsgen -replace -d idl Header.idl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/idl
  COMMENT "FastDDS IDL regeneration for OMS"
)

add_custom_target(show_sources
  COMMAND ${CMAKE_COMMAND} -E echo "=== OMS Sources ==="
  COMMAND ${CMAKE_COMMAND} -E echo "Core: ${CORE_SOURCES}"
  COMMAND ${CMAKE_COMMAND} -E echo "IDL : ${IDL_SOURCES}"
)

# Install main binary and configs
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

install(DIRECTORY config/
  DESTINATION etc/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.json"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/log4cxx.xml")
  install(FILES config/log4cxx.xml DESTINATION etc/${PROJECT_NAME})
endif()
