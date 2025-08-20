#pragma once

#include <cmath>
#include <stdexcept>

namespace Calculator {

/**
 * @brief Basic arithmetic operations for trading calculations
 */

inline double add(double a, double b) { return a + b; }

inline double subtract(double a, double b) { return a - b; }

inline double multiply(double a, double b) { return a * b; }

inline double divide(double a, double b) {
  if (std::abs(b) < 1e-10) {
    throw std::invalid_argument("Division by zero");
  }
  return a / b;
}

/**
 * @brief Financial calculation helpers for forex trading
 */

/**
 * @brief Calculate profit/loss for a forex position
 * @param entry_price Entry price of the position
 * @param exit_price Exit price of the position
 * @param quantity Position size (positive for long, negative for short)
 * @param pip_value Value per pip for the currency pair
 * @return Profit/loss amount
 */
inline double calculate_pnl(double entry_price, double exit_price,
                            double quantity, double pip_value = 1.0) {
  double price_diff = exit_price - entry_price;
  return price_diff * quantity * pip_value;
}

/**
 * @brief Calculate margin requirement for a forex position
 * @param price Current market price
 * @param quantity Position size
 * @param margin_rate Margin requirement rate (e.g., 0.02 for 2%)
 * @return Required margin amount
 */
inline double calculate_margin_requirement(double price, double quantity,
                                           double margin_rate) {
  return std::abs(price * quantity * margin_rate);
}

/**
 * @brief Calculate pip value for a currency pair
 * @param quantity Position size
 * @param account_currency_rate Exchange rate to account currency
 * @param pip_size Pip size (usually 0.0001 for major pairs, 0.01 for JPY pairs)
 * @return Value per pip
 */
inline double calculate_pip_value(double quantity, double account_currency_rate,
                                  double pip_size = 0.0001) {
  return quantity * pip_size * account_currency_rate;
}

/**
 * @brief Calculate leverage ratio
 * @param position_value Total value of positions
 * @param account_equity Account equity
 * @return Leverage ratio
 */
inline double calculate_leverage(double position_value, double account_equity) {
  if (account_equity <= 0) {
    throw std::invalid_argument("Account equity must be positive");
  }
  return position_value / account_equity;
}

/**
 * @brief Calculate required margin percentage
 * @param leverage Leverage ratio (e.g., 100 for 100:1)
 * @return Margin percentage
 */
inline double calculate_margin_percentage(double leverage) {
  if (leverage <= 0) {
    throw std::invalid_argument("Leverage must be positive");
  }
  return 1.0 / leverage;
}

/**
 * @brief Calculate position size based on risk amount
 * @param risk_amount Amount willing to risk
 * @param stop_loss_pips Stop loss in pips
 * @param pip_value Value per pip
 * @return Recommended position size
 */
inline double calculate_position_size(double risk_amount, double stop_loss_pips,
                                      double pip_value) {
  if (stop_loss_pips <= 0 || pip_value <= 0) {
    throw std::invalid_argument(
        "Stop loss pips and pip value must be positive");
  }
  return risk_amount / (stop_loss_pips * pip_value);
}

} // namespace Calculator
