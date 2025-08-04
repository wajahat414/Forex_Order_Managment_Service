#!/bin/bash

# Build script for Order Management Service

set -e

echo "=== Building Order Management Service ==="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
echo "Building project..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: build/OrderManagementService"
echo "Test Client: build/TestClient"
echo ""
echo "To run the service:"
echo "  cd build"
echo "  ./OrderManagementService ../config/users.json ../config/symbols.json"
echo ""
echo "Or to test with the client:"
echo "  ./TestClient"
