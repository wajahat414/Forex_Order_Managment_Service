#!/bin/bash

# Development setup script for Order Management Service
# This script helps set up the development environment

set -e

echo "=== Order Management Service Development Setup ==="

# Check if running on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS - checking for Homebrew..."
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Please install Homebrew first:"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    echo "Installing dependencies via Homebrew..."
    
    # Install CMake if not present
    if ! command -v cmake &> /dev/null; then
        brew install cmake
    fi
    
    # Install FastDDS dependencies
    echo "Note: FastDDS needs to be built from source on macOS."
    echo "Please follow the FastDDS installation guide:"
    echo "  https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html"
    echo ""
    echo "Quick installation steps:"
    echo "1. Install dependencies:"
    echo "   brew install cmake asio tinyxml2"
    echo ""
    echo "2. Build and install Fast-CDR:"
    echo "   git clone https://github.com/eProsima/Fast-CDR.git"
    echo "   cd Fast-CDR && mkdir build && cd build"
    echo "   cmake .. && make -j4 && sudo make install"
    echo ""
    echo "3. Build and install Fast-DDS:"
    echo "   git clone https://github.com/eProsima/Fast-DDS.git"
    echo "   cd Fast-DDS && mkdir build && cd build"
    echo "   cmake .. && make -j4 && sudo make install"
    echo ""
    
    # Install nlohmann/json
    if ! brew list nlohmann-json &> /dev/null; then
        brew install nlohmann-json
    fi
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Detected Linux - installing dependencies..."
    
    # Update package list
    sudo apt update
    
    # Install basic build tools
    sudo apt install -y build-essential cmake git
    
    # Try to install FastDDS from package manager (Ubuntu 22.04+)
    if apt-cache search libfastdds-dev | grep -q libfastdds-dev; then
        sudo apt install -y libfastdds-dev libfastcdr-dev
    else
        echo "FastDDS packages not available, need to build from source."
        echo "Please follow the installation guide for your distribution."
    fi
    
    # Install nlohmann/json
    sudo apt install -y nlohmann-json3-dev
else
    echo "Unsupported operating system: $OSTYPE"
    echo "Please install dependencies manually:"
    echo "  - CMake (>= 3.16)"
    echo "  - FastDDS and FastCDR"
    echo "  - nlohmann/json"
    exit 1
fi

echo ""
echo "=== Project Structure ==="
echo "The Order Management Service includes:"
echo "  - FastDDS-based order processing"
echo "  - Risk validation engine"
echo "  - JSON configuration system"
echo "  - Matching engine integration"
echo ""

echo "=== Next Steps ==="
echo "1. Ensure FastDDS is properly installed"
echo "2. Build the project:"
echo "   ./build.sh"
echo ""
echo "3. Run the service:"
echo "   cd build"
echo "   ./OrderManagementService"
echo ""
echo "4. Test with sample client:"
echo "   ./TestClient"
echo ""

echo "=== Configuration ==="
echo "Edit configuration files in the config/ directory:"
echo "  - config/users.json    (user risk parameters)"
echo "  - config/symbols.json  (symbol trading rules)"
echo ""

echo "Setup script completed!"
