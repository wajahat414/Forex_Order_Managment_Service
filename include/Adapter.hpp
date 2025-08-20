#pragma once

#include <ExecutionReport.hpp>
#include <NewOrderSingle.hpp>
#include <OrderMessage.hpp>
#include <OrderResponseReport.hpp>

using namespace OMS;
class Adapter {
public:
  static DistributedATS_NewOrderSingle::NewOrderSingle
  requestToNewOrderSingle(const OrderRequest &order);
  static OrderResponseReport executionReportToOrderReponseReport(
      const DistributedATS_ExecutionReport::ExecutionReport &executionReport);

private:
  static char convertOrderSideToChar(OrderSide side);
  static char convertOrderTypeToChar(OrderType type);
  static std::atomic<int32_t> sequence_counter_;

  /**
   * @brief Generate unique sequence number for FIX messages
   * Thread-safe sequence number generation for concurrent order processing
   */
  static int32_t generateSequenceNumber();

  /**
   * @brief Get current UTC timestamp in FIX format (YYYYMMDD-HH:MM:SS.sss)
   * Essential for financial message timestamping and audit trails
   */
  static u_int64_t getCurrentUTCTimestamp();

  static OrderStatus convertExecStatusToOrderStatus(char ord_status);

  static OrderSide convertCharToOrderSide(char side);
  static OrderType convertCharToOrderType(char ord_type);
};
