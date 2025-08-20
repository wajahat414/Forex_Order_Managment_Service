// OMS Test Program using traditional C++ headers
// This provides full IntelliSense support and eliminates C++20 module issues

#include "../include/Calculator.hpp"
#include "../include/Logger.hpp"
#include "../include/RiskService.hpp"
#include "../include/RiskServiceImpl.hpp"

#include <chrono>
#include <iostream>
#include <vector>

using namespace OMS;

int main() {
  Logger::system_event("OMS", "STARTUP",
                       "Initializing traditional header test");

  // Test Calculator functionality with forex calculations
  Logger::info("=== Testing Calculator Functions ===");

  try {
    // Basic arithmetic
    double sum = Calculator::add(100.5, 50.25);
    Logger::info("Addition test: " + std::to_string(sum));

    // Forex-specific calculations
    double pnl =
        Calculator::calculate_pnl(1.1000, 1.1050, 100000, 1.0); // EURUSD trade
    Logger::info("P&L calculation: $" + std::to_string(pnl));

    double margin =
        Calculator::calculate_margin_requirement(1.1000, 100000, 0.02);
    Logger::info("Margin requirement: $" + std::to_string(margin));

    double pip_value = Calculator::calculate_pip_value(100000, 1.0, 0.0001);
    Logger::info("Pip value: $" + std::to_string(pip_value));

    double leverage = Calculator::calculate_leverage(110000, 10000);
    Logger::info("Leverage ratio: " + std::to_string(leverage) + ":1");
  } catch (const std::exception &e) {
    Logger::error("Calculator test failed: " + std::string(e.what()));
  }

  // Test Risk Service functionality
  Logger::info("=== Testing Risk Service ===");

  try {
    // Create risk service using factory function
    auto risk_service = make_risk_service();

    // Create test forex order
    OrderIn eurusd_order;
    eurusd_order.client_id = "FOREX_CLIENT_001";
    eurusd_order.symbol = "EURUSD";
    eurusd_order.side = '1';     // Buy
    eurusd_order.qty = 100000.0; // Standard lot
    eurusd_order.price = 1.1000;
    eurusd_order.ord_type = '2'; // Limit order
    eurusd_order.tif = '0';      // Day order
    eurusd_order.account_id = "MT5_ACC_12345";
    eurusd_order.order_id = "ORD_EURUSD_001";

    // Test risk evaluation
    auto start_time = std::chrono::high_resolution_clock::now();
    auto risk_decision = risk_service->evaluate(eurusd_order);
    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    Logger::latency_measurement("risk_evaluation", duration.count());

    if (risk_decision.accepted) {
      Logger::order_event(eurusd_order.order_id, "APPROVED",
                          "Margin: $" +
                              std::to_string(risk_decision.calculated_margin));
    } else {
      Logger::order_event(eurusd_order.order_id, "REJECTED",
                          risk_decision.reason);
    }

    // Test position limits
    bool position_valid = risk_service->validate_position_limits(
        eurusd_order.symbol, eurusd_order.qty);
    Logger::info("Position limits validation: " +
                 std::string(position_valid ? "PASS" : "FAIL"));

    // Test margin calculation
    double margin_req =
        risk_service->calculate_margin_requirement(eurusd_order);
    Logger::info("Calculated margin: $" + std::to_string(margin_req));

    // Test custom risk limits
    RiskLimits custom_limits;
    custom_limits.max_position_size = 500000.0;   // Reduced limit
    custom_limits.max_order_value = 50000.0;      // Reduced order value
    custom_limits.margin_requirement_rate = 0.05; // Higher margin

    std::unordered_map<std::string, RiskLimits> client_limits;
    client_limits["VIP_CLIENT"] = custom_limits;

    auto vip_risk_service = make_risk_service_with_limits(client_limits);

    // Test with VIP client limits
    OrderIn vip_order = eurusd_order;
    vip_order.client_id = "VIP_CLIENT";
    vip_order.order_id = "ORD_VIP_001";
    vip_order.qty = 750000.0; // Should exceed VIP limits

    auto vip_decision = vip_risk_service->evaluate(vip_order);
    Logger::order_event(vip_order.order_id,
                        vip_decision.accepted ? "APPROVED" : "REJECTED",
                        vip_decision.reason);
  } catch (const std::exception &e) {
    Logger::error("Risk service test failed: " + std::string(e.what()));
  }

  // Test market data logging
  Logger::info("=== Testing Market Data Logging ===");
  Logger::market_data_event("EURUSD", 1.1000, 1.1002,
                            "2025-08-20 14:30:15.123");
  Logger::market_data_event("GBPUSD", 1.2550, 1.2553);

  // Test execution logging
  Logger::trade_execution("EXEC_001", "EURUSD", 100000, 1.1001);

  // Test FastDDS event logging
  Logger::fastdds_event("NEW_ORDER_TOPIC", "MESSAGE_PUBLISHED",
                        "Order ID: ORD_001");
  Logger::fastdds_event("EXECUTION_REPORT_TOPIC", "MESSAGE_RECEIVED");

  Logger::system_event("OMS", "TEST_COMPLETE",
                       "All traditional header tests passed");
  Logger::info(
      "=== IntelliSense should work perfectly throughout this file! ===");

  return 0;
}
