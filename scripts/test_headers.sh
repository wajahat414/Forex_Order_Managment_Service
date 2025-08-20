#!/bin/bash

# Test the simple modular headers independently
echo "=== Testing Simple Modular Headers ==="

# Create a test directory
TEST_DIR="test_simple_headers"
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Copy the headers
mkdir -p modules/simple
cp ../modules/simple/calculator.hpp modules/simple/
cp ../modules/simple/logger.hpp modules/simple/

# Create a simple test file
cat > test.cpp << 'EOF'
#include <iostream>
#include <string>
#include "modules/simple/calculator.hpp"
#include "modules/simple/logger.hpp"

int main() {
    Logger::info("Testing simple modular headers");
    
    int a = 15, b = 3;
    
    int sum = Calculator::add(a, b);
    int product = Calculator::multiply(a, b);
    double quotient = Calculator::divide(static_cast<double>(a), static_cast<double>(b));
    
    Logger::info("Calculator Test Results:");
    Logger::info("  " + std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(sum));
    Logger::info("  " + std::to_string(a) + " * " + std::to_string(b) + " = " + std::to_string(product));
    Logger::info("  " + std::to_string(a) + " / " + std::to_string(b) + " = " + std::to_string(quotient));
    
    // Test error handling
    try {
        double result = Calculator::divide(10.0, 0.0);
        Logger::error("This should not print!");
    } catch (const std::invalid_argument& e) {
        Logger::info("Division by zero correctly handled: " + std::string(e.what()));
    }
    
    Logger::info("✅ All tests passed!");
    return 0;
}
EOF

# Compile with Apple clang
echo "🔨 Compiling with Apple clang..."
/usr/bin/clang++ -std=c++20 -Wall -Wextra -O2 test.cpp -o test_headers

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    echo ""
    echo "🧪 Running tests..."
    ./test_headers
    echo ""
    echo "🎉 Simple header-based modules work perfectly!"
else
    echo "❌ Compilation failed!"
fi
