<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# Order Management Service (OMS) - Copilot Instructions

This is a C++ financial trading system component that implements an Order Management Service using FastDDS for communication.

## Project Overview

- **Language**: C++17
- **Communication**: FastDDS (Fast Data Distribution Service)
- **Purpose**: Listen to new order requests, validate against risk parameters, and route to matching engine
- **Domain**: Financial Trading / Forex System

## Architecture Components

1. **OrderListener**: Subscribes to "new_order_request" FastDDS topic
2. **RiskValidator**: Validates orders against user and symbol configurations
3. **OrderRouter**: Routes validated orders to matching engine via FastDDS
4. **OrderManagementService**: Main orchestrator class

## Key Features

- Real-time order processing via FastDDS pub/sub
- Comprehensive risk validation (user limits, symbol limits, parameter validation)
- JSON-based configuration for users and symbols
- Reliable message delivery with FastDDS
- Clean separation of concerns with modular design

## Development Guidelines

- Follow RAII principles for resource management
- Use smart pointers for memory management
- Implement proper error handling and logging
- Maintain thread safety for concurrent operations
- Follow FastDDS best practices for DDS communication
- Use JSON for configuration files with nlohmann/json library

## Configuration

- User configurations in `config/users.json`
- Symbol configurations in `config/symbols.json`
- Risk parameters include position limits, daily volume limits, margin requirements

## Build System

- CMake-based build system
- Automatic IDL code generation using fastddsgen
- Dependencies: FastDDS, FastCDR, nlohmann/json

When working with this codebase, focus on:

- Proper FastDDS participant, topic, and reader/writer lifecycle management
- Robust error handling for network communication
- Efficient order validation logic
- Clear separation between business logic and communication layers
