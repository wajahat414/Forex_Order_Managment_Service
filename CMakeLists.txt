cmake_minimum_required(VERSION 3.28)
project(oms_modular_monolith LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set paths for local FastDDS installation
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dds;${CMAKE_PREFIX_PATH}")

# Find required packages
find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)
find_package(nlohmann_json REQUIRED)
# find_package(Drogon REQUIRED)


set(LOG4CXX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/include")
set(LOG4CXX_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log4cxx/lib")
set(LOG4CXX_LIBRARY "${LOG4CXX_LIBRARY_DIR}/liblog4cxx.dylib")


if(NOT EXISTS ${LOG4CXX_LIBRARY})
    message(FATAL_ERROR "log4cxx library not found at ${LOG4CXX_LIBRARY}")
endif()

# Include directories
include_directories(include)
include_directories(common)
include_directories(idl/idl)
include_directories(idl/)
include_directories(dds/include)
include_directories(${LOG4CXX_INCLUDE_DIR})


file(GLOB_RECURSE CORE_SOURCES 
    "src/*.cpp"
    "src/*.cxx"
    "src/*.cc"
)

file(GLOB IDL_SOURCES 
    "idl/idl/*PubSubTypes.cxx"
    "idl/idl/*TypeObjectSupport.cxx"
)

message(STATUS "Discovered source files:")
foreach(SOURCE_FILE ${CORE_SOURCES})
    get_filename_component(FILENAME ${SOURCE_FILE} NAME)
    message(STATUS "  Core: ${FILENAME}")
endforeach()

message(STATUS "Discovered IDL files:")
foreach(IDL_FILE ${IDL_SOURCES})
    get_filename_component(FILENAME ${IDL_FILE} NAME)
    message(STATUS "  IDL: ${FILENAME}")
endforeach()



set(SOURCES ${CORE_SOURCES} ${IDL_SOURCES})

# Remove test_client.cpp from main executable if it exists in src/
list(FILTER SOURCES EXCLUDE REGEX ".*test_client\\.cpp$")



# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    fastdds
    fastcdr
    nlohmann_json::nlohmann_json
    ${LOG4CXX_LIBRARY}
)

# Set library paths for FastDDS
target_link_directories(${PROJECT_NAME} PRIVATE 
dds/lib
${LOG4CXX_LIBRARY_DIR}
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
    ${LOG4CXX_CFLAGS_OTHER}
)




# ✅ ENHANCED: TestClient with automatic source discovery
set(TEST_CLIENT_SOURCES)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/src/test_client.cpp")
    set(TEST_CLIENT_SOURCES test/src/test_client.cpp)
else()
    message(WARNING "test_client.cpp not found in src directory")
endif()

if(TEST_CLIENT_SOURCES)
    # TestClient needs minimal IDL dependencies
    set(TEST_CLIENT_IDL_SOURCES
        idl/idl/OrderMessagePubSubTypes.cxx
        idl/idl/OrderMessageTypeObjectSupport.cxx
    )

    add_executable(TestClient
        ${TEST_CLIENT_SOURCES}
        ${TEST_CLIENT_IDL_SOURCES}
    )

    target_link_libraries(TestClient
        fastdds
        fastcdr
        ${LOG4CXX_LIBRARY}  # Fixed: Link library file, not directory
    )

    target_include_directories(TestClient PRIVATE
        include
        common
        idl/idl
        dds/include
        ${LOG4CXX_INCLUDE_DIR}
    )

    target_link_directories(TestClient PRIVATE 
        dds/lib 
        ${LOG4CXX_LIBRARY_DIR}
    )

    target_compile_options(TestClient PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>
    )

    # Install TestClient only if built
    install(TARGETS TestClient RUNTIME DESTINATION bin)
endif()

# Install main OMS executable
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Install configuration files for OMS deployment
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.json"
)

# Install log configuration if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/log4cxx.xml")
    install(FILES config/log4cxx.xml
        DESTINATION etc/${PROJECT_NAME}
    )
endif()

# ✅ Custom target for IDL regeneration (development utility)
add_custom_target(regenerate_idl
    COMMAND echo "Regenerating FastDDS IDL files for OMS..."
    COMMAND fastddsgen -replace -d idl OrderMessage.idl
    COMMAND fastddsgen -replace -d idl NewOrderSingle.idl  
    COMMAND fastddsgen -replace -d idl ExecutionReport.idl
    COMMAND fastddsgen -replace -d idl Header.idl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/idl
    COMMENT "Regenerating FastDDS IDL files for financial trading system"
)

# ✅ Optional: Create development target that shows all discovered files
add_custom_target(show_sources
    COMMAND echo "=== OMS Source Files ==="
    COMMAND echo "Core sources: ${CORE_SOURCES}"
    COMMAND echo "IDL sources: ${IDL_SOURCES}"
    COMMENT "Display all discovered source files"
)


# ✅ FIXED: SimpleTest executable configuration with complete IDL dependencies
set(SIMPLE_TEST_SOURCES)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/src/simple_test.cpp")
    set(SIMPLE_TEST_SOURCES test/src/simple_test.cpp)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/simple_test.cpp")
    set(SIMPLE_TEST_SOURCES src/simple_test.cpp)
else()
    message(WARNING "simple_test.cpp not found in expected locations")
endif()

if(SIMPLE_TEST_SOURCES)
    # ✅ FIXED: Include all required IDL dependencies for NewOrderSingle
    set(SIMPLE_TEST_IDL_SOURCES
        idl/idl/OrderMessagePubSubTypes.cxx
        idl/idl/OrderMessageTypeObjectSupport.cxx
        # Add NewOrderSingle dependencies
        idl/idl/NewOrderSinglePubSubTypes.cxx
        idl/idl/NewOrderSingleTypeObjectSupport.cxx
        # Add Header dependencies (required by NewOrderSingle)
        idl/idl/HeaderPubSubTypes.cxx
        idl/idl/HeaderTypeObjectSupport.cxx
    )

    # Verify all required IDL files exist for SimpleTest
    foreach(IDL_FILE ${SIMPLE_TEST_IDL_SOURCES})
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${IDL_FILE}")
            message(WARNING "Required IDL file missing for SimpleTest: ${IDL_FILE}")
            message(STATUS "You may need to regenerate IDL files using: make regenerate_idl")
        endif()
    endforeach()

    add_executable(SimpleTest
        ${SIMPLE_TEST_SOURCES}
        ${SIMPLE_TEST_IDL_SOURCES}
    )

    # Link required libraries for SimpleTest
    target_link_libraries(SimpleTest
        fastdds
        fastcdr
        ${LOG4CXX_LIBRARY}
    )

    # Include directories for SimpleTest
    target_include_directories(SimpleTest PRIVATE
        include
        common
        idl/idl
        dds/include
        ${LOG4CXX_INCLUDE_DIR}
    )

    # Library search paths for SimpleTest
    target_link_directories(SimpleTest PRIVATE 
        dds/lib 
        ${LOG4CXX_LIBRARY_DIR}
    )

    # Compiler options for SimpleTest (financial system compliance)
    target_compile_options(SimpleTest PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter  # FastDDS generated code may have unused parameters
        $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )

    # Install SimpleTest for deployment
    install(TARGETS SimpleTest RUNTIME DESTINATION bin)

    message(STATUS "✅ SimpleTest executable configured with NewOrderSingle support")
endif()

