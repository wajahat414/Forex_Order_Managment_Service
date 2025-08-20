#include "RiskValidator.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RiskValidator::RiskValidator() {}

bool RiskValidator::initialize(const std::string &user_config_file,
                               const std::string &symbol_config_file) {
  if (!load_user_configs(user_config_file)) {
    std::cerr << "Failed to load user configurations" << std::endl;
    return false;
  }

  if (!load_symbol_configs(symbol_config_file)) {
    std::cerr << "Failed to load symbol configurations" << std::endl;
    return false;
  }

  std::cout << "RiskValidator initialized with " << user_configs_.size()
            << " users and " << symbol_configs_.size() << " symbols"
            << std::endl;
  return true;
}

bool RiskValidator::validate_order(const OrderRequest &order,
                                   std::string &rejection_reason) {
  // Basic order parameter validation
  if (!validate_order_parameters(order, rejection_reason)) {
    return false;
  }

  // Check user configuration
  auto user_it = user_configs_.find(order.user_id());
  if (user_it == user_configs_.end()) {
    rejection_reason = "User not found or not configured";
    return false;
  }

  // if (!validate_user_limits(order, user_it->second, rejection_reason))
  // {
  //     return false;
  // }

  // Check symbol configuration
  auto symbol_it = symbol_configs_.find(order.symbol());
  if (symbol_it == symbol_configs_.end()) {
    rejection_reason = "Symbol not found or not configured";
    return false;
  }

  if (!validate_symbol_limits(order, symbol_it->second, rejection_reason)) {
    return false;
  }

  return true;
}

bool RiskValidator::validate_order_parameters(const OrderRequest &order,
                                              std::string &reason) {
  if (order.order_id().empty()) {
    reason = "Order ID cannot be empty";
    return false;
  }

  if (order.symbol().empty()) {
    reason = "Symbol cannot be empty";
    return false;
  }

  if (order.quantity() <= 0) {
    reason = "Quantity must be positive";
    return false;
  }

  if (order.type() == OrderType::LIMIT && order.price() <= 0) {
    reason = "Limit orders must have positive price";
    return false;
  }

  if ((order.type() == OrderType::STOP ||
       order.type() == OrderType::STOP_LIMIT) &&
      order.stop_price() <= 0) {
    reason = "Stop orders must have positive stop price";
    return false;
  }

  return true;
}

bool RiskValidator::validate_user_limits(const OrderRequest &order,
                                         const UserConfig &user_config,
                                         std::string &reason) {
  if (!user_config.is_active) {
    reason = "User account is inactive";
    return false;
  }

  double order_value =
      order.quantity() *
      (order.type() == OrderType::MARKET ? 1.0 : order.price());

  if (order_value > user_config.max_position_size) {
    reason = "Order value exceeds maximum position size limit";
    return false;
  }

  if (order.quantity() > user_config.max_daily_volume) {
    reason = "Order quantity exceeds daily volume limit";
    return false;
  }

  double required_margin = order_value * user_config.margin_requirement;
  if (required_margin > user_config.available_balance) {
    reason = "Insufficient margin/balance for order";
    return false;
  }

  return true;
}

bool RiskValidator::validate_symbol_limits(const OrderRequest &order,
                                           const SymbolConfig &symbol_config,
                                           std::string &reason) {
  if (!symbol_config.is_tradeable) {
    reason = "Symbol is not tradeable";
    return false;
  }

  if (order.quantity() < symbol_config.min_quantity) {
    reason = "Order quantity below minimum allowed";
    return false;
  }

  if (order.quantity() > symbol_config.max_quantity) {
    reason = "Order quantity exceeds maximum allowed";
    return false;
  }

  double order_value =
      order.quantity() *
      (order.type() == OrderType::MARKET ? 1.0 : order.price());
  if (order_value > symbol_config.max_order_value) {
    reason = "Order value exceeds maximum allowed for symbol";
    return false;
  }

  // Validate price tick size for limit orders
  if (order.type() == OrderType::LIMIT && symbol_config.tick_size > 0) {
    double price_remainder = fmod(order.price(), symbol_config.tick_size);
    if (price_remainder > 1e-8) { // Small epsilon for floating point comparison
      reason = "Order price does not conform to tick size";
      return false;
    }
  }

  return true;
}

bool RiskValidator::load_user_configs(const std::string &config_file) {
  try {
    std::ifstream file(config_file);
    if (!file.is_open()) {
      std::cerr << "Cannot open user config file: " << config_file << std::endl;
      return false;
    }

    json j;
    file >> j;

    for (const auto &user_json : j["users"]) {
      UserConfig config;
      config.user_id = user_json["user_id"];
      config.max_position_size = user_json["max_position_size"];
      config.max_daily_volume = user_json["max_daily_volume"];
      config.available_balance = user_json["available_balance"];
      config.margin_requirement = user_json["margin_requirement"];
      config.is_active = user_json["is_active"];

      user_configs_[config.user_id] = config;
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading user configs: " << e.what() << std::endl;
    return false;
  }
}

bool RiskValidator::load_symbol_configs(const std::string &config_file) {
  try {
    std::ifstream file(config_file);
    if (!file.is_open()) {
      std::cerr << "Cannot open symbol config file: " << config_file
                << std::endl;
      return false;
    }

    json j;
    file >> j;

    for (const auto &symbol_json : j["symbols"]) {
      SymbolConfig config;
      config.symbol = symbol_json["symbol"];
      config.min_quantity = symbol_json["min_quantity"];
      config.max_quantity = symbol_json["max_quantity"];
      config.tick_size = symbol_json["tick_size"];
      config.margin_rate = symbol_json["margin_rate"];
      config.is_tradeable = symbol_json["is_tradeable"];
      config.max_order_value = symbol_json["max_order_value"];

      symbol_configs_[config.symbol] = config;
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading symbol configs: " << e.what() << std::endl;
    return false;
  }
}

void RiskValidator::update_user_config(const std::string &user_id,
                                       const UserConfig &config) {
  user_configs_[user_id] = config;
}

void RiskValidator::update_symbol_config(const std::string &symbol,
                                         const SymbolConfig &config) {
  symbol_configs_[symbol] = config;
}
