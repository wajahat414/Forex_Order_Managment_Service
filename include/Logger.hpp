#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace Logger {

// Thread-safe logging for the OMS
namespace {
std::mutex log_mutex;

std::string get_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}
} // namespace

/**
 * @brief Thread-safe logging functions for the OMS
 */

inline void info(const std::string &message) {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::cout << "[INFO] [" << get_timestamp() << "] " << message << std::endl;
}

inline void error(const std::string &message) {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::cerr << "[ERROR] [" << get_timestamp() << "] " << message << std::endl;
}

inline void debug(const std::string &message) {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::cout << "[DEBUG] [" << get_timestamp() << "] " << message << std::endl;
}

inline void warning(const std::string &message) {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::cout << "[WARN] [" << get_timestamp() << "] " << message << std::endl;
}

/**
 * @brief Trading-specific logging functions
 */

inline void order_event(const std::string &order_id, const std::string &event,
                        const std::string &details = "") {
  std::ostringstream oss;
  oss << "ORDER[" << order_id << "] " << event;
  if (!details.empty()) {
    oss << " - " << details;
  }
  info(oss.str());
}

inline void risk_event(const std::string &client_id, const std::string &symbol,
                       const std::string &event,
                       const std::string &details = "") {
  std::ostringstream oss;
  oss << "RISK[" << client_id << ":" << symbol << "] " << event;
  if (!details.empty()) {
    oss << " - " << details;
  }
  warning(oss.str());
}

inline void trade_execution(const std::string &trade_id,
                            const std::string &symbol, double quantity,
                            double price) {
  std::ostringstream oss;
  oss << "EXECUTION[" << trade_id << "] " << symbol << " Qty:" << quantity
      << " Price:" << std::fixed << std::setprecision(5) << price;
  info(oss.str());
}

inline void market_data_event(const std::string &symbol, double bid, double ask,
                              const std::string &timestamp = "") {
  std::ostringstream oss;
  oss << "MARKET[" << symbol << "] Bid:" << std::fixed << std::setprecision(5)
      << bid << " Ask:" << ask;
  if (!timestamp.empty()) {
    oss << " Time:" << timestamp;
  }
  debug(oss.str());
}

inline void system_event(const std::string &component, const std::string &event,
                         const std::string &status) {
  std::ostringstream oss;
  oss << "SYSTEM[" << component << "] " << event << " Status:" << status;
  info(oss.str());
}

inline void fastdds_event(const std::string &topic, const std::string &event,
                          const std::string &details = "") {
  std::ostringstream oss;
  oss << "FASTDDS[" << topic << "] " << event;
  if (!details.empty()) {
    oss << " - " << details;
  }
  debug(oss.str());
}

/**
 * @brief Performance logging for latency monitoring
 */
inline void latency_measurement(const std::string &operation,
                                double latency_us) {
  std::ostringstream oss;
  oss << "LATENCY[" << operation << "] " << std::fixed << std::setprecision(2)
      << latency_us << " μs";
  debug(oss.str());
}

/**
 * @brief Template function for formatted logging
 */
template <typename... Args>
inline void info_formatted(const std::string &format, Args &&...args) {
  std::ostringstream oss;
  oss << format;
  info(oss.str());
}

} // namespace Logger
