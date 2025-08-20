#pragma once

#include "RiskService.hpp"
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace OMS {

/**
 * @brief Implementation of risk service for forex trading
 *
 * This class provides comprehensive risk management for the OMS,
 * including position limits, margin calculations, and real-time
 * risk monitoring for forex trading operations.
 */
class RiskServiceImpl final : public IRiskService {
private:
  mutable std::mutex limits_mutex_;
  mutable std::mutex daily_volume_mutex_;

  // Risk limits per client
  std::unordered_map<std::string, RiskLimits> client_limits_;

  // Daily volume tracking
  std::unordered_map<std::string, double> daily_volumes_;
  std::unordered_map<std::string, std::chrono::system_clock::time_point>
      last_reset_;

  // Helper methods
  void reset_daily_counters_if_needed(const std::string &client_id);
  bool is_new_trading_day(const std::string &client_id) const;
  bool validate_order_parameters(const OrderIn &order) const;
  bool validate_symbol_format(const std::string &symbol) const;

public:
  /**
   * @brief Constructor - initializes default risk limits
   */
  RiskServiceImpl();

  /**
   * @brief Constructor with custom default limits
   * @param default_limits Default risk limits to use
   */
  explicit RiskServiceImpl(const RiskLimits &default_limits);

  // IRiskService interface implementation
  RiskDecision evaluate(const OrderIn &order) override;
  bool validate_position_limits(const std::string &symbol,
                                double new_quantity) override;
  double calculate_margin_requirement(const OrderIn &order) override;
  RiskLimits get_risk_limits(const std::string &client_id) override;

  /**
   * @brief Update risk limits for a specific client
   * @param client_id Client identifier
   * @param limits New risk limits
   * @return true if updated successfully
   */
  bool update_risk_limits(const std::string &client_id,
                          const RiskLimits &limits);

  /**
   * @brief Validate daily volume limits
   * @param client_id Client identifier
   * @param order_volume Volume of the new order
   * @return true if within daily limits
   */
  bool validate_daily_volume(const std::string &client_id, double order_volume);

  /**
   * @brief Get current daily volume for a client
   * @param client_id Client identifier
   * @return Current daily volume
   */
  double get_daily_volume(const std::string &client_id) const;

  /**
   * @brief Reset daily counters for a client (for testing)
   * @param client_id Client identifier
   */
  void reset_daily_counters(const std::string &client_id);
};

// Inline implementations for performance-critical functions

inline RiskServiceImpl::RiskServiceImpl() {
  // Initialize default risk limits for forex trading
  RiskLimits default_limits;
  default_limits.max_position_size = 1000000.0;  // 1M units max position
  default_limits.max_daily_volume = 5000000.0;   // 5M units daily volume
  default_limits.margin_requirement_rate = 0.02; // 2% margin
  default_limits.max_order_value = 100000.0;     // $100k max order
  default_limits.daily_loss_limit = 50000.0;     // $50k daily loss limit
  default_limits.allow_hedging = true;           // Allow hedging

  client_limits_["DEFAULT"] = default_limits;
}

inline RiskServiceImpl::RiskServiceImpl(const RiskLimits &default_limits) {
  client_limits_["DEFAULT"] = default_limits;
}

inline bool
RiskServiceImpl::validate_symbol_format(const std::string &symbol) const {
  // Basic forex symbol validation (e.g., EURUSD, GBPJPY)
  return !symbol.empty() && symbol.length() >= 6 && symbol.length() <= 8;
}

inline bool
RiskServiceImpl::validate_order_parameters(const OrderIn &order) const {
  return !order.client_id.empty() && !order.symbol.empty() && order.qty > 0.0 &&
         (order.side == '1' || order.side == '2') &&
         (order.ord_type == '1' || order.ord_type == '2');
}

// Factory function implementations
inline std::unique_ptr<IRiskService> make_risk_service() {
  return std::make_unique<RiskServiceImpl>();
}

inline std::unique_ptr<IRiskService> make_risk_service_with_limits(
    const std::unordered_map<std::string, RiskLimits> &custom_limits) {
  auto service = std::make_unique<RiskServiceImpl>();
  for (const auto &[client_id, limits] : custom_limits) {
    service->update_risk_limits(client_id, limits);
  }
  return service;
}

} // namespace OMS
