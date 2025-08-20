#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace OMS {

/**
 * @brief Risk decision result for order validation
 */
struct RiskDecision {
  bool accepted{false};
  std::string reason;
  double calculated_margin{0.0};
  double position_impact{0.0};
};

/**
 * @brief Order input structure for risk evaluation
 * Following FIX protocol standards for financial trading
 */
struct OrderIn {
  std::string client_id;
  std::string symbol;
  char side{'1'}; // FIX: '1' = Buy, '2' = Sell
  double qty{0.0};
  double price{0.0};
  char ord_type{'2'}; // FIX: '1' = Market, '2' = Limit
  char tif{'0'};      // FIX: '0' = Day, '1' = GTC

  // Additional forex trading fields
  double stop_price{0.0};
  std::string account_id;
  std::string order_id;
};

/**
 * @brief Risk limits configuration for forex trading
 */
struct RiskLimits {
  double max_position_size{1000000.0};  // Maximum position per symbol
  double max_daily_volume{5000000.0};   // Maximum daily trading volume
  double margin_requirement_rate{0.02}; // 2% margin requirement
  double max_order_value{100000.0};     // Maximum single order value
  double daily_loss_limit{50000.0};     // Maximum daily loss limit
  bool allow_hedging{true};             // Allow hedging positions
};

/**
 * @brief Base interface for risk validation in forex trading systems
 *
 * This interface provides core risk management functionality for
 * the Order Management Service (OMS) including position limits,
 * margin requirements, and trading rule validation.
 */
class IRiskService {
public:
  virtual ~IRiskService() = default;

  /**
   * @brief Evaluate risk for a new order
   * @param order The order to evaluate
   * @return RiskDecision indicating acceptance or rejection
   */
  virtual RiskDecision evaluate(const OrderIn &order) = 0;

  /**
   * @brief Validate position limits for a symbol
   * @param symbol Trading symbol (e.g., "EURUSD")
   * @param new_quantity Proposed position size
   * @return true if within limits, false otherwise
   */
  virtual bool validate_position_limits(const std::string &symbol,
                                        double new_quantity) = 0;

  /**
   * @brief Calculate margin requirement for an order
   * @param order The order to calculate margin for
   * @return Required margin amount
   */
  virtual double calculate_margin_requirement(const OrderIn &order) = 0;

  /**
   * @brief Get risk limits for a specific client
   * @param client_id Client identifier
   * @return Risk limits configuration
   */
  virtual RiskLimits get_risk_limits(const std::string &client_id) = 0;
};

/**
 * @brief Factory function to create a default risk service
 * @return Unique pointer to risk service implementation
 */
std::unique_ptr<IRiskService> make_risk_service();

/**
 * @brief Factory function to create risk service with custom limits
 * @param custom_limits Map of client_id to risk limits
 * @return Unique pointer to risk service implementation
 */
std::unique_ptr<IRiskService> make_risk_service_with_limits(
    const std::unordered_map<std::string, RiskLimits> &custom_limits);

} // namespace OMS
