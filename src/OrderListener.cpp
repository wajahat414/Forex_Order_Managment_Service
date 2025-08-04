#include "OrderListener.hpp"
#include "RiskValidator.hpp"
#include "OrderRouter.hpp"
#include <iostream>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

OrderListener::OrderListener(std::shared_ptr<RiskValidator> risk_validator,
                             std::shared_ptr<OrderRouter> order_router)
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), risk_validator_(risk_validator), order_router_(order_router)
{
}

OrderListener::~OrderListener()
{
    cleanup();
}

bool OrderListener::init()
{
    // Create participant
    DomainParticipantQos pqos;
    pqos.name("OrderManagementService_Participant");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }

    // Register the type
    type_.reset(new OrderRequestPubSubType());
    if (type_.register_type(participant_) != RETCODE_OK)
    {
        std::cerr << "Failed to register OrderRequest type" << std::endl;
        return false;
    }

    // Create topic
    TopicQos tqos;
    topic_ = participant_->create_topic("new_order_request", type_.get_type_name(), tqos);
    if (topic_ == nullptr)
    {
        std::cerr << "Failed to create topic" << std::endl;
        return false;
    }

    // Create subscriber
    SubscriberQos sqos;
    subscriber_ = participant_->create_subscriber(sqos);
    if (subscriber_ == nullptr)
    {
        std::cerr << "Failed to create subscriber" << std::endl;
        return false;
    }

    // Create DataReader
    DataReaderQos rqos;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;

    reader_ = subscriber_->create_datareader(topic_, rqos, this);
    if (reader_ == nullptr)
    {
        std::cerr << "Failed to create DataReader" << std::endl;
        return false;
    }

    std::cout << "OrderListener initialized successfully" << std::endl;
    return true;
}

void OrderListener::cleanup()
{
    if (participant_ != nullptr)
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
            reader_ = nullptr;
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
            subscriber_ = nullptr;
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
            topic_ = nullptr;
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }
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
    }
}

void OrderListener::on_subscription_matched(DataReader *reader, const SubscriptionMatchedStatus &info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched with publisher" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched from publisher" << std::endl;
    }
}

void OrderListener::process_order(const OrderRequest &order)
{
    std::string rejection_reason;

    // Validate order through risk validator
    if (!risk_validator_->validate_order(order, rejection_reason))
    {
        std::cout << "Order " << order.order_id() << " rejected: " << rejection_reason << std::endl;

        // Send rejection response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::REJECTED);
        response.message(rejection_reason);
        response.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count());

        order_router_->send_order_response(response);
        return;
    }

    // Order passed validation, route to matching engine
    if (order_router_->route_to_matching_engine(order))
    {
        std::cout << "Order " << order.order_id() << " routed to matching engine" << std::endl;

        // Send validation success response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::VALIDATED);
        response.message("Order validated and routed to matching engine");
        response.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count());

        order_router_->send_order_response(response);
    }
    else
    {
        std::cerr << "Failed to route order " << order.order_id() << " to matching engine" << std::endl;

        // Send routing failure response
        OrderResponse response;
        response.order_id(order.order_id());
        response.status(OrderStatus::REJECTED);
        response.message("Failed to route order to matching engine");
        response.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count());

        order_router_->send_order_response(response);
    }
}
