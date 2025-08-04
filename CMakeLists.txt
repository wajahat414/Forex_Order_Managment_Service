cmake_minimum_required(VERSION 3.16)
project(OrderManagementService VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set paths for local FastDDS installation
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dds;${CMAKE_PREFIX_PATH}")

# Find required packages
find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(include)
include_directories(idl/idl)
include_directories(dds/include)

# Source files
set(SOURCES
    src/main.cpp
    src/OrderManagementService.cpp
    src/OrderListener.cpp
    src/RiskValidator.cpp
    src/OrderRouter.cpp
    idl/idl/OrderMessagePubSubTypes.cxx
    idl/idl/OrderMessageTypeObjectSupport.cxx
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    fastdds
    fastcdr
    nlohmann_json::nlohmann_json
)

# Set library paths for FastDDS
target_link_directories(${PROJECT_NAME} PRIVATE dds/lib)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Create test client executable
add_executable(TestClient
    src/test_client.cpp
    idl/idl/OrderMessagePubSubTypes.cxx
    idl/idl/OrderMessageTypeObjectSupport.cxx
)

target_link_libraries(TestClient
    fastdds
    fastcdr
)

# Set library paths for TestClient
target_link_directories(TestClient PRIVATE dds/lib)

# Install targets
install(TARGETS ${PROJECT_NAME} TestClient
    RUNTIME DESTINATION bin
)

# Install config files
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.json"
)
