#!/bin/bash
# Build script for OMS

set -e

echo "=== Building Order Management Service ==="

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Configure project
echo "Configuring OMS project..."
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build main OMS executable
echo "Building OMS executable..."
cmake --build . --target OMS -j

# Build tests
echo "Building OMS tests..."
cmake --build . --target HeaderTest -j

echo "✅ OMS build completed successfully!"
echo ""
echo "Available executables:"
echo "  ./build/OMS - Main Order Management Service"
echo "  ./build/HeaderTest - Risk validation and order processing tests"
echo ""
echo "To run OMS:"
echo "  ./build/OMS config/users.json config/symbols.json"
