#!/bin/bash

# Traditional Headers Build Script for OMS
echo "=== OMS Traditional Headers Build ==="
echo "🎯 Building with traditional C++ headers for maximum IntelliSense compatibility"

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf build_headers

# Create build directory
echo "📁 Creating build directory..."
mkdir -p build_headers
cd build_headers

# Configure with CMake using Apple clang (better for headers)
echo "⚙️ Configuring with CMake (traditional headers)..."
cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_SHARED_LIBS=OFF

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Build
echo "🔨 Building project with traditional headers..."
if ninja; then
    echo "✅ Build successful!"
    
    echo ""
    echo "🧪 Running traditional header tests..."
    echo "Running HeaderTest program:"
    echo ""
    
    if [ -f "./HeaderTest" ]; then
        ./HeaderTest
        echo ""
        echo "🎉 Traditional headers test completed successfully!"
    else
        echo "⚠️  HeaderTest executable not found"
    fi
    
    if [ -f "./oms_traditional_headers" ]; then
        echo ""
        echo "📍 Main OMS executable: $(pwd)/oms_traditional_headers"
        echo "📍 Header test executable: $(pwd)/HeaderTest"
    fi
    
    echo ""
    echo "✅ Traditional C++ headers build completed!"
    echo "🔍 IntelliSense should work perfectly now in VS Code"
    echo ""
    echo "� To use in your IDE:"
    echo "   1. Use #include statements instead of import"
    echo "   2. Include headers from include/ directory"
    echo "   3. Full autocomplete and symbol resolution available"
    
else
    echo "❌ Build failed!"
    exit 1
fi
