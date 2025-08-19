#!/bin/bash

# Run script for Order Management Service with optimized settings for macOS

echo "=== Starting Order Management Service ==="

# Set environment variables to reduce FastDDS warnings on macOS
export FASTDDS_DEFAULT_PROFILES_FILE="config/fastdds_profile.xml"
export RMW_IMPLEMENTATION=rmw_fastrtps_cpp

# Disable thread affinity warnings (macOS doesn't support this well)
export FASTDDS_BUILTIN_TRANSPORTS=LARGE_DATA

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Running build first..."
    ./build.sh
fi

cd build

# Check if config files exist
if [ ! -f "../config/users.json" ]; then
    echo "Error: users.json not found in config directory"
    exit 1
fi

if [ ! -f "../config/symbols.json" ]; then
    echo "Error: symbols.json not found in config directory"
    exit 1
fi

echo "Starting Order Management Service..."
echo "Press Ctrl+C to stop"
echo ""

# Run the service with proper library path
DYLD_LIBRARY_PATH="../dds/lib:$DYLD_LIBRARY_PATH" ./OrderManagementService ../config/users.json ../config/symbols.json ../config/fastdds_profile.xml
