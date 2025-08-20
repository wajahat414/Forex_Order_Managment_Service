# Utilities.cmake - Utility targets and custom commands for OMS

# ===================================================================
# IDL Generation Utilities
# ===================================================================

add_custom_target(regenerate_idl
  COMMAND echo "Regenerating FastDDS IDL files..."
  COMMAND fastddsgen -replace -d idl OrderMessage.idl
  COMMAND fastddsgen -replace -d idl NewOrderSingle.idl
  COMMAND fastddsgen -replace -d idl ExecutionReport.idl
  COMMAND fastddsgen -replace -d idl Header.idl
  COMMAND fastddsgen -replace -d idl BusinessMessageReject.idl
  COMMAND fastddsgen -replace -d idl OrderCancelRequest.idl
  COMMAND fastddsgen -replace -d idl OrderCancelReject.idl
  COMMAND fastddsgen -replace -d idl OrderCancelReplaceRequest.idl
  COMMAND fastddsgen -replace -d idl OrderMassCancelRequest.idl
  COMMAND fastddsgen -replace -d idl OrderMassCancelReport.idl
  COMMAND fastddsgen -replace -d idl OrderMassStatusRequest.idl
  COMMAND fastddsgen -replace -d idl OrderResponseReport.idl
  COMMAND fastddsgen -replace -d idl MarketDataRequest.idl
  COMMAND fastddsgen -replace -d idl MarketDataSnapshotFullRefresh.idl
  COMMAND fastddsgen -replace -d idl MarketDataIncrementalRefresh.idl
  COMMAND fastddsgen -replace -d idl SecurityListRequest.idl
  COMMAND fastddsgen -replace -d idl SecurityList.idl
  COMMAND fastddsgen -replace -d idl Logon.idl
  COMMAND fastddsgen -replace -d idl Logout.idl
  COMMAND fastddsgen -replace -d idl Heartbeat.idl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/idl
  COMMENT "FastDDS IDL regeneration for OMS"
)

# ===================================================================
# Development Utilities
# ===================================================================

add_custom_target(show_sources
  COMMAND ${CMAKE_COMMAND} -E echo "=== OMS Source Files ==="
  COMMAND ${CMAKE_COMMAND} -E echo "Core Sources:"
  COMMAND ${CMAKE_COMMAND} -E echo "${CORE_SOURCES}"
  COMMAND ${CMAKE_COMMAND} -E echo ""
  COMMAND ${CMAKE_COMMAND} -E echo "IDL Sources:"
  COMMAND ${CMAKE_COMMAND} -E echo "${IDL_SOURCES}"
  COMMENT "Display source file information"
)

add_custom_target(show_config
  COMMAND ${CMAKE_COMMAND} -E echo "=== OMS Build Configuration ==="
  COMMAND ${CMAKE_COMMAND} -E echo "Build Type: ${CMAKE_BUILD_TYPE}"
  COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
  COMMAND ${CMAKE_COMMAND} -E echo "C++ Standard: ${CMAKE_CXX_STANDARD}"
  COMMAND ${CMAKE_COMMAND} -E echo "Install Prefix: ${CMAKE_INSTALL_PREFIX}"
  COMMENT "Display build configuration"
)

# ===================================================================
# Clean Utilities
# ===================================================================

add_custom_target(clean_all
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
  COMMENT "Deep clean of build artifacts"
)

# ===================================================================
# Development Convenience
# ===================================================================

# Quick rebuild target
add_custom_target(quick_build
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
  COMMENT "Quick parallel build"
)

# Format code (if clang-format is available)
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  add_custom_target(format
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs ${CLANG_FORMAT_EXECUTABLE} -i
    COMMENT "Formatting source code with clang-format"
  )
endif()
