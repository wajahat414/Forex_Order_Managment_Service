#include "OrderManagementService.hpp"
#include <chrono>
#include <iostream>
#include <thread>

OrderManagementService::OrderManagementService() : running_(false) {}

OrderManagementService::~OrderManagementService() {
  if (running_) {
    stop();
  }
  cleanup();
}

bool OrderManagementService::initialize(
    const std::string &user_config_file, const std::string &symbol_config_file,
    OrderManagmentService::Application &application) {
  std::cout << "Initializing Order Management Service..." << std::endl;

  // Initialize risk validator
  risk_validator_ = std::make_shared<RiskValidator>();
  if (!risk_validator_->initialize(user_config_file, symbol_config_file)) {
    std::cerr << "Failed to initialize risk validator" << std::endl;
    return false;
  }

  // Initialize order router
  order_router_ = std::make_shared<OrderRouter>(application);

  // Initialize order listener
  order_listener_ =
      std::make_shared<OrderListener>(risk_validator_, order_router_);
  if (!order_listener_->init()) {
    std::cerr << "Failed to initialize order listener" << std::endl;
    return false;
  }

  std::cout << "Order Management Service initialized successfully" << std::endl;
  return true;
}

void OrderManagementService::start() {
  if (running_) {
    std::cout << "Service is already running" << std::endl;
    return;
  }

  running_ = true;
  std::cout << "Order Management Service started. Listening for orders..."
            << std::endl;

  // Keep the service running
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void OrderManagementService::stop() {
  if (!running_) {
    std::cout << "Service is not running" << std::endl;
    return;
  }

  running_ = false;
  std::cout << "Order Management Service stopped" << std::endl;
}

void OrderManagementService::cleanup() {
  if (order_listener_) {
    order_listener_->cleanup();
    order_listener_.reset();
  }

  risk_validator_.reset();
}
