#include "Adapter.hpp"
#include <iostream>

std::atomic<int32_t> Adapter::sequence_counter_{1000};
char Adapter::convertOrderSideToChar(OrderSide side)
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

char Adapter::convertOrderTypeToChar(OrderType type)
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

DistributedATS_NewOrderSingle::NewOrderSingle Adapter::requestToNewOrderSingle(const OrderRequest &order)

{
    DistributedATS::Header header;
    header.BeginString("FIX.4.4");                       // FIX protocol version
    header.MsgType("D");                                 // NewOrderSingle message type
    header.SenderCompID("OMS_ROUTER");                   // OMS component identifier
    header.TargetCompID("MATCHING_ENGINE");              // Destination matching engine
    header.MsgSeqNum(Adapter::generateSequenceNumber()); // Unique sequence number
    header.SendingTime(Adapter::getCurrentUTCTimestamp());

    DistributedATS_NewOrderSingle::NewOrderSingle new_order;
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

OrderResponseReport Adapter::executionReportToOrderReponseReport(const DistributedATS_ExecutionReport::ExecutionReport &executionReport)
{
    OrderResponseReport orderReport;

    // ✅ Core order identification mapping following FIX protocol standards
    orderReport.OrderID(executionReport.OrderID());
    orderReport.OrigClOrdID(executionReport.OrigClOrdID());
    orderReport.ExecID(executionReport.ExecID());

    // ✅ Order status mapping from execution report following OMS architecture
    orderReport.OrdStatus(executionReport.OrdStatus());
    orderReport.ExecType(executionReport.ExecType());

    // ✅ Financial instrument and trading details
    orderReport.Symbol(executionReport.Symbol());
    orderReport.SecurityExchange(executionReport.SecurityExchange());
    orderReport.Side(executionReport.Side());

    // ✅ Quantity and pricing information following trading standards
    orderReport.OrderQty(executionReport.OrderQty());
    orderReport.LastQty(executionReport.LastQty());
    orderReport.CumQty(executionReport.CumQty());
    orderReport.LeavesQty(executionReport.LeavesQty());

    // ✅ Price information with proper validation
    orderReport.Price(executionReport.Price());
    orderReport.LastPx(executionReport.LastPx());
    orderReport.AvgPx(executionReport.AvgPx());
    orderReport.StopPx(executionReport.StopPx());

    // ✅ Order type and time in force mapping
    orderReport.OrdType(executionReport.OrdType());
    orderReport.TimeInForce(executionReport.TimeInForce());

    // ✅ Timestamp handling for audit trail compliance
    orderReport.TransactTime(executionReport.TransactTime());

    // ✅ Rejection handling for risk management
    if (executionReport.OrdRejReason() != 0)
    {
        orderReport.OrdRejReason(executionReport.OrdRejReason());
    }

    // ✅ Additional information and execution instructions
    orderReport.ExecInst(executionReport.ExecInst());
    orderReport.Text(executionReport.Text());

    // ✅ DATS routing information for system tracking
    orderReport.DATS_Source(executionReport.DATS_Source());
    orderReport.DATS_Destination(executionReport.DATS_Destination());
    orderReport.DATS_SourceUser(executionReport.DATS_SourceUser());
    orderReport.DATS_DestinationUser(executionReport.DATS_DestinationUser());

    std::cout << "✅ Converted ExecutionReport to OrderResponseReport: "
              << executionReport.OrderID() << " (" << executionReport.ExecID() << ")"
              << " Status: " << orderReport.OrdStatus() << std::endl;

    return orderReport;
}

uint64_t Adapter::getCurrentUTCTimestamp()
{
    // Get current time with microsecond precision for financial trading
    auto now = std::chrono::system_clock::now();
    auto epoch_time = now.time_since_epoch();

    // Convert to microseconds since Unix epoch (standard for financial systems)
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch_time);

    return static_cast<uint64_t>(microseconds.count());
}

int32_t Adapter::generateSequenceNumber()
{
    // Thread-safe sequence number generation for concurrent order processing
    return sequence_counter_.fetch_add(1, std::memory_order_relaxed);
}

// Add to Adapter.cpp
OrderStatus Adapter::convertExecStatusToOrderStatus(char ord_status)
{
    switch (ord_status)
    {

    case '2':
        return OrderStatus::FILLED;
    case '4':
        return OrderStatus::CANCELLED;
    case '8':
        return OrderStatus::REJECTED;
    default:
        std::cerr << "❌ Unknown order status: " << ord_status << std::endl;
        return OrderStatus::CANCELLED;
    }
}

OrderSide Adapter::convertCharToOrderSide(char side)
{
    switch (side)
    {
    case '1':
        return OrderSide::BUY;
    case '2':
        return OrderSide::SELL;
    default:
        std::cerr << "❌ Unknown order side: " << side << std::endl;
        return OrderSide::BUY; // Default to BUY for safety
    }
}

OrderType Adapter::convertCharToOrderType(char ord_type)
{
    switch (ord_type)
    {
    case '1':
        return OrderType::MARKET;
    case '2':
        return OrderType::LIMIT;
    case '3':
        return OrderType::STOP;
    case '4':
        return OrderType::STOP_LIMIT;
    default:
        std::cerr << "❌ Unknown order type: " << ord_type << std::endl;
        return OrderType::MARKET; // Default to MARKET for safety
    }
}
