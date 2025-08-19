#include "OrderListener.hpp"
#include "RiskValidator.hpp"
#include "OrderRouter.hpp"
#include <iostream>
#include <thread>
#include <BasicDomainParticipant.h>

OrderListener::OrderListener(std::shared_ptr<RiskValidator> risk_validator,
                             std::shared_ptr<OrderRouter> order_router)
    : participant_(nullptr), topic_tuple_(nullptr), reader_tuple_(nullptr), risk_validator_(risk_validator), order_router_(order_router)
{
}

OrderListener::~OrderListener()
{
    cleanup();
}

bool OrderListener::init()
{
    try
    {
        std::cout << "Initializing OrderListener..." << std::endl;

        participant_ = std::make_shared<distributed_ats_utils::basic_domain_participant>(
            DEFAULT_DOMAIN_ID, OMS_PARTICIPANT_NAME);
        if (!participant_->create_subscriber())
        {
            std::cerr << "Failed to create subscriber for OrderListener" << std::endl;
            LOG4CXX_ERROR(logger, "Failed to create DomainParticipant");
            return false;
        }
        std::cout << "✅ Created subscriber" << std::endl;

        topic_tuple_ = participant_->make_topic<OrderRequestPubSubType, OrderRequestPubSubType>(
            NEW_ORDER_REQUEST_TOPIC_NAME);
        if (topic_tuple_ == nullptr)
        {
            std::cerr << "Failed to create topic tuple" << std::endl;
            LOG4CXX_ERROR(logger, "Failed to create topic Tuple");
            return false;
        }

        std::cout << "✅ Created topic: " << NEW_ORDER_REQUEST_TOPIC_NAME << std::endl;

        // Create data tuple reader with this listener

        reader_tuple_ = participant_->make_data_reader_tuple<OrderRequestPubSubType>(
            topic_tuple_, this);
        if (reader_tuple_ == nullptr)
        {
            std::cerr << "Failed to create data reader tuple" << std::endl;
            LOG4CXX_ERROR(logger, "Failed to create data reader tuple");
            return false;
        }
        std::cout << "✅ Created DataReader with listener" << std::endl;

        std::cout << "OrderListener initialized successfully" << std::endl;
        LOG4CXX_INFO(logger, "Order Listener initialized successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception during OrderListener initialization: " << e.what() << std::endl;
        LOG4CXX_ERROR(logger, "Exception during OrderListener initialization: " << e.what());
        cleanup();
        return false;
    }
}

void OrderListener::cleanup()
{

    reader_tuple_.reset();
    topic_tuple_.reset();
    participant_.reset();
    LOG4CXX_INFO(logger, "OrderListener resources cleaned up");
}

void OrderListener::on_data_available(DataReader *reader)
{
    OrderRequest order;
    SampleInfo info;

    while (reader->read_next_sample(&order, &info) == RETCODE_OK)
    {

        if (info.valid_data)
        {
            std::cout << "Received new order: " << order.order_id()
                      << " for symbol: " << order.symbol() << std::endl;
            process_order(order);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void OrderListener::on_subscription_matched(eprosima::fastdds::dds::DataReader *reader, const eprosima::fastdds::dds::SubscriptionMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched with publisher" << std::endl;
        LOG4CXX_INFO(logger, "Subscriber matched with publisher");
        std::cout << "Current count: " << info.current_count << ", Total count: " << info.total_count << std::endl;
        LOG4CXX_INFO(logger, "Current count: " << info.current_count << ", Total count: " << info.total_count);
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched from publisher" << std::endl;
        LOG4CXX_INFO(logger, "Subscriber unmatched from publisher");
        std::cout << "Current count: " << info.current_count << ", Total count: " << info.total_count << std::endl;
        LOG4CXX_INFO(logger, "Current count: " << info.current_count << ", Total count: " << info.total_count);
    }
}

void OrderListener::process_order(const OrderRequest &order)
{
    std::string rejection_reason;

    auto current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    LOG4CXX_DEBUG(logger, "Processing order: " << order.order_id() << " at timestamp: " << current_timestamp << " from user: " << order.user_id());
    // Validate order through risk validator
    if (!risk_validator_->validate_order(order, rejection_reason))
    {
        std::cout << "Order " << order.order_id() << " rejected: " << rejection_reason << std::endl;
        LOG4CXX_WARN(logger, "Order " << order.order_id() << " rejected: " << rejection_reason);

        // Send rejection response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::REJECTED);
        response.message(rejection_reason);
        response.timestamp(current_timestamp);

        return;
    }

    // Order passed validation, route to matching engine
    if (order_router_->route_to_matching_engine(order))
    {
        std::cout << "Order " << order.order_id() << " routed to matching engine" << std::endl;
        LOG4CXX_INFO(logger, "Order " << order.order_id() << " routed to matching engine");

        // Send validation success response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::ROUTED);
        response.message("Order validated and routed to matching engine");
        response.timestamp(current_timestamp);
    }
    else
    {
        std::cerr << "Failed to route order " << order.order_id() << " to matching engine" << std::endl;

        LOG4CXX_ERROR(logger, "Failed to route order " << order.order_id() << " to matching engine");
        // Send routing failure response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::REJECTED);
        response.message("Failed to route order to matching engine");
        response.timestamp(current_timestamp);
    }
}
