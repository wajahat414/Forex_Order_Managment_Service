#include "Adapter.hpp"

#include <iostream>

std::atomic<int32_t> OrderAdapter::sequence_counter_{1000};
char OrderAdapter::convertOrderSideToChar(OrderSide side)
{
    switch (side)
    {
    case OrderSide::BUY:
        return '1';
    case OrderSide::SELL:
        return '2';
    default:
        std::cerr << "Unknown order side: " << static_cast<int>(side) << std::endl;
        return '0'; // Return invalid side character
    }
}

char OrderAdapter::convertOrderTypeToChar(OrderType type)
{
    switch (type)
    {
    case OrderType::MARKET:
        return '1';
    case OrderType::LIMIT:
        return '2';
    case OrderType::STOP:
        return '3';
    case OrderType::STOP_LIMIT:
        return '4';
    default:
        std::cerr << "Unknown order type: " << static_cast<int>(type) << std::endl;
        return '0'; // Return invalid type character
    }
}

NewOrderSingle OrderAdapter::requestToNewOrderSingle(const OrderRequest &order)

{
    Header header;
    header.BeginString("FIX.4.4");                            // FIX protocol version
    header.MsgType("D");                                      // NewOrderSingle message type
    header.SenderCompID("OMS_ROUTER");                        // OMS component identifier
    header.TargetCompID("MATCHING_ENGINE");                   // Destination matching engine
    header.MsgSeqNum(OrderAdapter::generateSequenceNumber()); // Unique sequence number
    header.SendingTime(OrderAdapter::getCurrentUTCTimestamp());

    NewOrderSingle new_order;
    new_order.fix_header(header);
    new_order.ClOrdID(order.order_id());
    new_order.Symbol(order.symbol());

    char side_char = convertOrderSideToChar(order.side());
    if (side_char == '0')
    {
        return new_order; // Return empty order on error
    }
    new_order.Side(side_char);

    if (order.quantity() <= 0.0)
    {
        std::cerr << "❌ Invalid quantity for order: " << order.order_id() << std::endl;

        throw std::invalid_argument("Invalid quantity in order: " + order.order_id());
    }

    new_order.OrderQty(order.quantity());
    if (order.type() != OrderType::MARKET && order.price() <= 0.0)
    {
        std::cerr << "❌ Invalid price for non-market order: " << order.order_id() << std::endl;
        throw std::invalid_argument("Invalid price for non-market order: " + order.order_id());
    }
    new_order.Price(order.price());

    char type_char = convertOrderTypeToChar(order.type());
    if (type_char == '0')
    {
        return new_order; // Return empty order on error
    }
    new_order.OrdType(type_char);

    // ✅ Enhanced timestamp handling for financial systems
    // ✅ Enhanced timestamp handling - use original order timestamp or current time
    uint64_t transaction_time = (order.timestamp() > 0) ? order.timestamp() : getCurrentUTCTimestamp();
    new_order.TransactTime(transaction_time);

    new_order.TimeInForce('0');

    new_order.Text("Order routed from OMS via OrderRouter");

    std::cout << "✅ Converted OrderRequest to NewOrderSingle: " << order.order_id()
              << " for " << order.symbol() << std::endl;

    return new_order;
}

uint64_t OrderAdapter::getCurrentUTCTimestamp()
{
    // Get current time with microsecond precision for financial trading
    auto now = std::chrono::system_clock::now();
    auto epoch_time = now.time_since_epoch();

    // Convert to microseconds since Unix epoch (standard for financial systems)
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch_time);

    return static_cast<uint64_t>(microseconds.count());
}

int32_t OrderAdapter::generateSequenceNumber()
{
    // Thread-safe sequence number generation for concurrent order processing
    return sequence_counter_.fetch_add(1, std::memory_order_relaxed);
}
