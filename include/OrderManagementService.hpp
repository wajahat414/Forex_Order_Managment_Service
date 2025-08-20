#pragma once

#include "OrderListener.hpp"
#include "OrderRouter.hpp"
#include "RiskValidator.hpp"
#include <atomic>
#include <memory>

class OrderManagementService {
public:
  OrderManagementService();
  ~OrderManagementService();

  bool initialize(const std::string &user_config_file,
                  const std::string &symbol_config_file,
                  OrderManagmentService::Application &application);
  void start();
  void stop();
  bool is_running() const { return running_; }

private:
  std::shared_ptr<RiskValidator> risk_validator_;
  std::shared_ptr<OrderRouter> order_router_;
  std::shared_ptr<OrderListener> order_listener_;

  std::atomic<bool> running_;

  void cleanup();
};
