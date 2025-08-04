#include "OrderRouter.hpp"
#include <iostream>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

OrderRouter::OrderRouter()
    : participant_(nullptr), publisher_(nullptr), matching_engine_topic_(nullptr), response_topic_(nullptr), matching_engine_writer_(nullptr), response_writer_(nullptr)
{
}

OrderRouter::~OrderRouter()
{
    cleanup();
}

bool OrderRouter::initialize()
{
    // Create participant
    DomainParticipantQos pqos;
    pqos.name("OrderRouter_Participant");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (participant_ == nullptr)
    {
        std::cerr << "Failed to create DomainParticipant for OrderRouter" << std::endl;
        return false;
    }

    // Register types
    order_type_.reset(new OrderRequestPubSubType());
    if (order_type_.register_type(participant_) != RETCODE_OK)
    {
        std::cerr << "Failed to register OrderRequest type" << std::endl;
        return false;
    }

    response_type_.reset(new OrderResponsePubSubType());
    if (response_type_.register_type(participant_) != RETCODE_OK)
    {
        std::cerr << "Failed to register OrderResponse type" << std::endl;
        return false;
    }

    // Create publisher
    PublisherQos pub_qos;
    publisher_ = participant_->create_publisher(pub_qos);
    if (publisher_ == nullptr)
    {
        std::cerr << "Failed to create publisher" << std::endl;
        return false;
    }

    // Setup publishers
    if (!setup_matching_engine_publisher() || !setup_response_publisher())
    {
        return false;
    }

    std::cout << "OrderRouter initialized successfully" << std::endl;
    return true;
}

bool OrderRouter::setup_matching_engine_publisher()
{
    // Create topic for matching engine
    TopicQos tqos;
    matching_engine_topic_ = participant_->create_topic(
        "matching_engine_orders", order_type_.get_type_name(), tqos);

    if (matching_engine_topic_ == nullptr)
    {
        std::cerr << "Failed to create matching engine topic" << std::endl;
        return false;
    }

    // Create DataWriter for matching engine
    DataWriterQos wqos;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = VOLATILE_DURABILITY_QOS;

    matching_engine_writer_ = publisher_->create_datawriter(matching_engine_topic_, wqos);
    if (matching_engine_writer_ == nullptr)
    {
        std::cerr << "Failed to create DataWriter for matching engine" << std::endl;
        return false;
    }

    return true;
}

bool OrderRouter::setup_response_publisher()
{
    // Create topic for order responses
    TopicQos tqos;
    response_topic_ = participant_->create_topic(
        "order_responses", response_type_.get_type_name(), tqos);

    if (response_topic_ == nullptr)
    {
        std::cerr << "Failed to create response topic" << std::endl;
        return false;
    }

    // Create DataWriter for responses
    DataWriterQos wqos;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = VOLATILE_DURABILITY_QOS;

    response_writer_ = publisher_->create_datawriter(response_topic_, wqos);
    if (response_writer_ == nullptr)
    {
        std::cerr << "Failed to create DataWriter for responses" << std::endl;
        return false;
    }

    return true;
}

void OrderRouter::cleanup()
{
    if (participant_ != nullptr)
    {
        if (matching_engine_writer_ != nullptr)
        {
            publisher_->delete_datawriter(matching_engine_writer_);
            matching_engine_writer_ = nullptr;
        }
        if (response_writer_ != nullptr)
        {
            publisher_->delete_datawriter(response_writer_);
            response_writer_ = nullptr;
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
            publisher_ = nullptr;
        }
        if (matching_engine_topic_ != nullptr)
        {
            participant_->delete_topic(matching_engine_topic_);
            matching_engine_topic_ = nullptr;
        }
        if (response_topic_ != nullptr)
        {
            participant_->delete_topic(response_topic_);
            response_topic_ = nullptr;
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
        participant_ = nullptr;
    }
}

bool OrderRouter::route_to_matching_engine(const OrderRequest &order)
{
    if (matching_engine_writer_ == nullptr)
    {
        std::cerr << "Matching engine writer not initialized" << std::endl;
        return false;
    }

    ReturnCode_t ret = matching_engine_writer_->write(&order);
    if (ret != RETCODE_OK)
    {
        std::cerr << "Failed to write order to matching engine topic" << std::endl;
        return false;
    }

    std::cout << "Order " << order.order_id() << " successfully routed to matching engine" << std::endl;
    return true;
}

bool OrderRouter::send_order_response(const OrderResponse &response)
{
    if (response_writer_ == nullptr)
    {
        std::cerr << "Response writer not initialized" << std::endl;
        return false;
    }

    ReturnCode_t ret = response_writer_->write(&response);
    if (ret != RETCODE_OK)
    {
        std::cerr << "Failed to write order response" << std::endl;
        return false;
    }

    std::cout << "Response sent for order " << response.order_id()
              << " with status: " << static_cast<int>(response.status()) << std::endl;
    return true;
}
