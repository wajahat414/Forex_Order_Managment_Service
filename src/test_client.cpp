#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

class OrderClient
{
private:
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *topic_;
    DataWriter *writer_;
    TypeSupport type_;

public:
    OrderClient() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr) {}

    ~OrderClient()
    {
        cleanup();
    }

    bool init()
    {
        // Create participant
        DomainParticipantQos pqos;
        pqos.name("OrderClient_Participant");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

        if (participant_ == nullptr)
        {
            std::cerr << "Failed to create DomainParticipant" << std::endl;
            return false;
        }

        // Register type
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

        // Create publisher
        PublisherQos pub_qos;
        publisher_ = participant_->create_publisher(pub_qos);
        if (publisher_ == nullptr)
        {
            std::cerr << "Failed to create publisher" << std::endl;
            return false;
        }

        // Create DataWriter
        DataWriterQos wqos;
        wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

        writer_ = publisher_->create_datawriter(topic_, wqos);
        if (writer_ == nullptr)
        {
            std::cerr << "Failed to create DataWriter" << std::endl;
            return false;
        }

        std::cout << "OrderClient initialized successfully" << std::endl;
        return true;
    }

    void cleanup()
    {
        if (participant_ != nullptr)
        {
            if (writer_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
                writer_ = nullptr;
            }
            if (publisher_ != nullptr)
            {
                participant_->delete_publisher(publisher_);
                publisher_ = nullptr;
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

    bool send_order(const std::string &order_id, const std::string &symbol,
                    OrderSide side, double quantity, double price = 0.0)
    {
        OrderRequest order;
        order.client_id("test_client");
        order.order_id(order_id);
        order.symbol(symbol);
        order.side(side);
        order.type(price > 0 ? OrderType::LIMIT : OrderType::MARKET);
        order.quantity(quantity);
        order.price(price);
        order.stop_price(0.0);
        order.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count());
        order.user_id("user001");

        ReturnCode_t ret = writer_->write(&order);
        if (ret != RETCODE_OK)
        {
            std::cerr << "Failed to send order" << std::endl;
            return false;
        }

        std::cout << "Sent order: " << order_id << " for " << symbol
                  << " (" << (side == OrderSide::BUY ? "BUY" : "SELL")
                  << " " << quantity << ")" << std::endl;
        return true;
    }
};

int main()
{
    std::cout << "=== Order Client Test ===" << std::endl;

    OrderClient client;
    if (!client.init())
    {
        return 1;
    }

    // Wait a bit for discovery
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Send test orders
    client.send_order("ORDER_001", "EURUSD", OrderSide::BUY, 10000.0, 1.1850);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    client.send_order("ORDER_002", "GBPUSD", OrderSide::SELL, 5000.0, 1.2750);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    client.send_order("ORDER_003", "USDJPY", OrderSide::BUY, 15000.0); // Market order
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test invalid order (inactive user)
    OrderRequest invalid_order;
    invalid_order.client_id("test_client");
    invalid_order.order_id("ORDER_004");
    invalid_order.symbol("EURUSD");
    invalid_order.side(OrderSide::BUY);
    invalid_order.type(OrderType::LIMIT);
    invalid_order.quantity(10000.0);
    invalid_order.price(1.1850);
    invalid_order.stop_price(0.0);
    invalid_order.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count());
    invalid_order.user_id("user003"); // inactive user

    std::cout << "Test orders sent. Check OMS console for processing results." << std::endl;

    return 0;
}
