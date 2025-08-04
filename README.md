# Order Management Service (OMS)

A high-performance Order Management Service for financial trading systems built with C++ and FastDDS.

## Overview

The Order Management Service (OMS) is a critical component of a forex trading system that:

- **Listens** to new order requests via FastDDS topic `new_order_request`
- **Validates** orders against user and global symbol configurations for risk management
- **Routes** validated orders to the matching engine for execution
- **Responds** with order status updates

## Architecture

```
┌─────────────────┐    FastDDS     ┌─────────────────┐
│   Trading       │  new_order_    │     Order       │
│   Clients       │─────request────▶│  Management     │
│                 │                 │    Service      │
└─────────────────┘                 └─────────────────┘
                                            │
                                            ▼
                                    ┌───────────────┐
                                    │ Risk          │
                                    │ Validation    │
                                    └───────────────┘
                                            │
                                            ▼
┌─────────────────┐    FastDDS     ┌─────────────────┐
│   Matching      │◀─matching_eng──│   Order Router  │
│    Engine       │   ine_orders   │                 │
└─────────────────┘                └─────────────────┘
```

## Components

### OrderListener

- Subscribes to `new_order_request` FastDDS topic
- Handles incoming order messages
- Triggers validation and routing pipeline

### RiskValidator

- Validates order parameters (quantity, price, etc.)
- Checks user-specific limits (position size, daily volume, balance)
- Validates symbol-specific constraints (tradeable status, tick size, limits)
- Loads configuration from JSON files

### OrderRouter

- Routes validated orders to matching engine via `matching_engine_orders` topic
- Publishes order responses to `order_responses` topic
- Manages FastDDS publishers and data writers

### OrderManagementService

- Main orchestrator class
- Manages component lifecycle
- Handles service startup/shutdown

## Configuration

### User Configuration (`config/users.json`)

```json
{
  "users": [
    {
      "user_id": "user001",
      "max_position_size": 1000000.0,
      "max_daily_volume": 5000000.0,
      "available_balance": 500000.0,
      "margin_requirement": 0.05,
      "is_active": true
    }
  ]
}
```

### Symbol Configuration (`config/symbols.json`)

```json
{
  "symbols": [
    {
      "symbol": "EURUSD",
      "min_quantity": 1000.0,
      "max_quantity": 10000000.0,
      "tick_size": 0.00001,
      "margin_rate": 0.033,
      "is_tradeable": true,
      "max_order_value": 5000000.0
    }
  ]
}
```

## Dependencies

- **FastDDS**: Real-time publish-subscribe communication
- **FastCDR**: Common Data Representation for serialization
- **nlohmann/json**: JSON parsing and manipulation
- **CMake**: Build system (minimum version 3.16)
- **C++17**: Standard library features

## Building

### Prerequisites

```bash
# Install FastDDS
sudo apt update
sudo apt install libfastdds-dev libfastcdr-dev

# Install nlohmann/json
sudo apt install nlohmann-json3-dev

# Or build from source following FastDDS documentation
```

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Build Options

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Running

```bash
# Default configuration
./OrderManagementService

# Custom configuration files
./OrderManagementService /path/to/users.json /path/to/symbols.json
```

## Message Types

### OrderRequest

```cpp
struct OrderRequest {
    string client_id;
    string order_id;
    string symbol;
    OrderSide side;        // BUY, SELL
    OrderType type;        // MARKET, LIMIT, STOP, STOP_LIMIT
    double quantity;
    double price;
    double stop_price;
    long long timestamp;
    string user_id;
};
```

### OrderResponse

```cpp
struct OrderResponse {
    string order_id;
    OrderStatus status;    // NEW, VALIDATED, REJECTED, ROUTED, FILLED, CANCELLED
    string message;
    long long timestamp;
};
```

## FastDDS Topics

- **new_order_request**: Incoming orders from trading clients
- **matching_engine_orders**: Validated orders sent to matching engine
- **order_responses**: Status updates and responses

## Risk Validation Rules

### Order Parameter Validation

- Non-empty order ID and symbol
- Positive quantity
- Valid price for limit orders
- Valid stop price for stop orders

### User Limit Validation

- Account must be active
- Order value within position size limits
- Quantity within daily volume limits
- Sufficient margin/balance available

### Symbol Limit Validation

- Symbol must be tradeable
- Quantity within min/max bounds
- Order value within symbol limits
- Price conforms to tick size (for limit orders)

## Development

### Code Structure

```
├── src/                 # Source files
├── include/            # Header files
├── idl/                # FastDDS IDL definitions
├── config/             # Configuration files
├── CMakeLists.txt      # Build configuration
└── README.md
```

### Adding New Features

1. Update IDL files for new message types
2. Regenerate C++ code: `fastddsgen -replace OrderMessage.idl`
3. Implement business logic in appropriate components
4. Update configuration schemas if needed

## Logging and Monitoring

The service provides console logging for:

- Order reception and processing
- Validation results and rejection reasons
- Routing status and matching engine communication
- Service lifecycle events

## Error Handling

- Graceful handling of malformed messages
- Configuration file parsing errors
- FastDDS communication failures
- Resource cleanup on shutdown

## Performance Considerations

- Zero-copy message passing with FastDDS
- Efficient JSON parsing with nlohmann/json
- Lock-free data structures where possible
- Configurable QoS policies for different reliability requirements

## Testing

### Unit Tests

```bash
# Build tests
cmake -DBUILD_TESTS=ON ..
make tests

# Run tests
./tests/unit_tests
```

### Integration Tests

- Test with FastDDS discovery and communication
- Validate end-to-end order processing
- Performance benchmarking

## Deployment

### Docker Deployment

```dockerfile
FROM ubuntu:22.04
# Install dependencies and build service
COPY . /app
WORKDIR /app
RUN cmake . && make
CMD ["./OrderManagementService"]
```

### System Requirements

- Linux/Unix environment
- Minimum 2GB RAM
- Network connectivity for FastDDS
- File system access for configuration

## License

This project is proprietary software for financial trading systems.

## Support

For technical support and questions, contact the development team.
