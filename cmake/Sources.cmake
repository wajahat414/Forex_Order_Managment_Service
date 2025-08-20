# Sources.cmake - Source file discovery and organization for OMS

# ===================================================================
# Core OMS Sources
# ===================================================================

# Discover all core source files
file(GLOB_RECURSE CORE_SOURCES
  "src/*.cpp"
  "src/*.cxx" 
  "src/*.cc"
)

# Exclude test files from main executable (they have their own main functions)
list(REMOVE_ITEM CORE_SOURCES 
  "${CMAKE_CURRENT_SOURCE_DIR}/src/module_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/header_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_client.cpp"
)

# Exclude modules smoke test when modules are disabled
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/modules_smoke_test\\.cpp$")

# ===================================================================
# FastDDS IDL Generated Sources
# ===================================================================

file(GLOB IDL_SOURCES
  "idl/idl/*PubSubTypes.cxx"
  "idl/idl/*TypeObjectSupport.cxx"
)

# ===================================================================
# Test Sources (separate from main executable)
# ===================================================================

set(HEADER_TEST_SOURCES
  "src/header_test.cpp"
  "src/RiskServiceImpl.cpp"
)

set(MODULE_TEST_SOURCES
  "src/module_test.cpp"
)

# ===================================================================
# Source Diagnostics
# ===================================================================

function(print_source_summary)
  message(STATUS "Source file summary:")
  message(STATUS "  Core sources (${list_length} files):")
  list(LENGTH CORE_SOURCES core_count)
  foreach(f ${CORE_SOURCES})
    get_filename_component(fname ${f} NAME)
    message(STATUS "    ${fname}")
  endforeach()
  
  message(STATUS "  IDL sources:")
  foreach(f ${IDL_SOURCES})
    get_filename_component(fname ${f} NAME)
    message(STATUS "    ${fname}")
  endforeach()
endfunction()

# Call diagnostics if requested
if(OMS_VERBOSE_BUILD)
  print_source_summary()
endif()
