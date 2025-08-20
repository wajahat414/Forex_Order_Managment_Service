# Tests.cmake - Test executable configuration for OMS

# ===================================================================
# Header Test Executable
# ===================================================================

add_executable(HeaderTest ${HEADER_TEST_SOURCES})

target_include_directories(HeaderTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR}/idl/idl
  ${CMAKE_CURRENT_SOURCE_DIR}/dds/include
  ${LOG4CXX_INCLUDE_DIR}
)

target_link_libraries(HeaderTest PRIVATE
  fastdds
  fastcdr
  nlohmann_json::nlohmann_json
  ${LOG4CXX_LIBRARY}
)

# Compiler options for tests
target_compile_options(HeaderTest PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
  $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# macOS runtime paths for tests
if(APPLE)
  set_target_properties(HeaderTest PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path/../lib;@loader_path/../lib;${CMAKE_CURRENT_SOURCE_DIR}/dds/lib;${LOG4CXX_LIBRARY_DIR}"
    MACOSX_RPATH ON
  )
endif()

message(STATUS "HeaderTest configured for traditional C++ headers")

# ===================================================================
# Module Test Executable (if needed in future)
# ===================================================================

# Uncomment if you want to re-enable module testing
# add_executable(ModuleTest ${MODULE_TEST_SOURCES})
# target_include_directories(ModuleTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modules)
# target_link_libraries(ModuleTest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
# message(STATUS "ModuleTest configured")

# ===================================================================
# Test Utilities
# ===================================================================

# Custom target to run all tests
add_custom_target(run_tests
  COMMAND echo "Running OMS tests..."
  COMMAND $<TARGET_FILE:HeaderTest>
  DEPENDS HeaderTest
  COMMENT "Running OMS test suite"
  VERBATIM
)

# Test-specific install rules (optional)
install(TARGETS HeaderTest 
  RUNTIME DESTINATION bin/tests
  OPTIONAL
)
