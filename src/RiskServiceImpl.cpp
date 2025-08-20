#include "../include/RiskServiceImpl.hpp"
#include "../include/Logger.hpp"
#include <algorithm>
#include <regex>

namespace OMS {

RiskDecision RiskServiceImpl::evaluate(const OrderIn &order) {
  Logger::debug("Evaluating risk for order: " + order.order_id);

  // Basic parameter validation
  if (!validate_order_parameters(order)) {
    return {false, "Invalid order parameters", 0.0, 0.0};
  }

  // Symbol format validation
  if (!validate_symbol_format(order.symbol)) {
    return {false, "Invalid symbol format: " + order.symbol, 0.0, 0.0};
  }

  // Get client risk limits
  auto limits = get_risk_limits(order.client_id);

  // Validate position limits
  if (!validate_position_limits(order.symbol, order.qty)) {
    Logger::risk_event(order.client_id, order.symbol, "POSITION_LIMIT_EXCEEDED",
                       "Requested: " + std::to_string(order.qty) +
                           " Max: " + std::to_string(limits.max_position_size));
    return {false, "Position limit exceeded for " + order.symbol, 0.0,
            order.qty};
  }

  // Calculate order value and validate
  double order_value = std::abs(order.qty * order.price);
  if (order_value > limits.max_order_value) {
    Logger::risk_event(order.client_id, order.symbol, "ORDER_VALUE_EXCEEDED",
                       "Value: $" + std::to_string(order_value) + " Max: $" +
                           std::to_string(limits.max_order_value));
    return {false,
            "Order value exceeds maximum allowed: $" +
                std::to_string(limits.max_order_value),
            0.0, order_value};
  }

  // Validate daily volume limits
  if (!validate_daily_volume(order.client_id, order_value)) {
    Logger::risk_event(order.client_id, order.symbol, "DAILY_VOLUME_EXCEEDED",
                       "Order Value: $" + std::to_string(order_value));
    return {false, "Daily volume limit exceeded", 0.0, order_value};
  }

  // Calculate margin requirement
  double margin_required = calculate_margin_requirement(order);

  // Additional forex-specific validations
  if (order.ord_type == '2' && order.price <= 0) {
    return {false, "Invalid price for limit order", margin_required, order.qty};
  }

  // Stop price validation for stop orders
  if (order.stop_price > 0) {
    if ((order.side == '1' && order.stop_price >= order.price) ||
        (order.side == '2' && order.stop_price <= order.price)) {
      return {false, "Invalid stop price for order direction", margin_required,
              order.qty};
    }
  }

  Logger::order_event(order.order_id, "RISK_APPROVED",
                      "Symbol: " + order.symbol +
                          " Qty: " + std::to_string(order.qty) + " Margin: $" +
                          std::to_string(margin_required));

  return {true, "Order approved", margin_required, order.qty};
}

bool RiskServiceImpl::validate_position_limits(const std::string &symbol,
                                               double new_quantity) {
  auto limits =
      get_risk_limits("DEFAULT"); // In production, get by symbol/client
  return std::abs(new_quantity) <= limits.max_position_size;
}

double RiskServiceImpl::calculate_margin_requirement(const OrderIn &order) {
  auto limits = get_risk_limits(order.client_id);
  double position_value = std::abs(order.qty * order.price);
  return position_value * limits.margin_requirement_rate;
}

RiskLimits RiskServiceImpl::get_risk_limits(const std::string &client_id) {
  std::lock_guard<std::mutex> lock(limits_mutex_);
  auto it = client_limits_.find(client_id);
  return it != client_limits_.end() ? it->second : client_limits_["DEFAULT"];
}

bool RiskServiceImpl::update_risk_limits(const std::string &client_id,
                                         const RiskLimits &limits) {
  std::lock_guard<std::mutex> lock(limits_mutex_);
  client_limits_[client_id] = limits;
  Logger::info("Updated risk limits for client: " + client_id);
  return true;
}

bool RiskServiceImpl::validate_daily_volume(const std::string &client_id,
                                            double order_volume) {
  std::lock_guard<std::mutex> lock(daily_volume_mutex_);

  reset_daily_counters_if_needed(client_id);

  auto limits = get_risk_limits(client_id);
  double current_volume = daily_volumes_[client_id];

  return (current_volume + order_volume) <= limits.max_daily_volume;
}

double RiskServiceImpl::get_daily_volume(const std::string &client_id) const {
  std::lock_guard<std::mutex> lock(daily_volume_mutex_);
  auto it = daily_volumes_.find(client_id);
  return it != daily_volumes_.end() ? it->second : 0.0;
}

void RiskServiceImpl::reset_daily_counters(const std::string &client_id) {
  std::lock_guard<std::mutex> lock(daily_volume_mutex_);
  daily_volumes_[client_id] = 0.0;
  last_reset_[client_id] = std::chrono::system_clock::now();
}

void RiskServiceImpl::reset_daily_counters_if_needed(
    const std::string &client_id) {
  if (is_new_trading_day(client_id)) {
    daily_volumes_[client_id] = 0.0;
    last_reset_[client_id] = std::chrono::system_clock::now();
    Logger::debug("Reset daily counters for client: " + client_id);
  }
}

bool RiskServiceImpl::is_new_trading_day(const std::string &client_id) const {
  auto it = last_reset_.find(client_id);
  if (it == last_reset_.end()) {
    return true; // First time, consider it a new day
  }

  auto now = std::chrono::system_clock::now();
  auto last_reset_time = it->second;

  // Simple check: if more than 24 hours have passed
  auto duration =
      std::chrono::duration_cast<std::chrono::hours>(now - last_reset_time);
  return duration.count() >= 24;
}

} // namespace OMS
