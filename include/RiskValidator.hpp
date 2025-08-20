#pragma once

#include "OrderMessage.hpp"
#include <memory>
#include <string>
#include <unordered_map>

struct UserConfig {
  std::string user_id;
  double max_position_size;
  double max_daily_volume;
  double available_balance;
  double margin_requirement;
  bool is_active;
};

struct SymbolConfig {
  std::string symbol;
  double min_quantity;
  double max_quantity;
  double tick_size;
  double margin_rate;
  bool is_tradeable;
  double max_order_value;
};

class RiskValidator {
public:
  RiskValidator();
  ~RiskValidator() = default;

  bool initialize(const std::string &user_config_file,
                  const std::string &symbol_config_file);

  bool validate_order(const OrderRequest &order, std::string &rejection_reason);

  void update_user_config(const std::string &user_id, const UserConfig &config);
  void update_symbol_config(const std::string &symbol,
                            const SymbolConfig &config);

private:
  std::unordered_map<std::string, UserConfig> user_configs_;
  std::unordered_map<std::string, SymbolConfig> symbol_configs_;

  bool load_user_configs(const std::string &config_file);
  bool load_symbol_configs(const std::string &config_file);

  bool validate_user_limits(const OrderRequest &order,
                            const UserConfig &user_config, std::string &reason);
  bool validate_symbol_limits(const OrderRequest &order,
                              const SymbolConfig &symbol_config,
                              std::string &reason);
  bool validate_order_parameters(const OrderRequest &order,
                                 std::string &reason);
};
