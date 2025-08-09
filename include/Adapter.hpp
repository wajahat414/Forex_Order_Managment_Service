#pragma once

#include <OrderMessage.hpp>
#include <NewOrderSingle.hpp>

class OrderAdapter
{
public:
    static DistributedATS_NewOrderSingle::NewOrderSingle requestToNewOrderSingle(const OrderRequest &order);

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
};
