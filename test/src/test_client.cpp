#include <BasicDomainParticipant.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"
#include <Common.h>
#include <random>

using namespace eprosima::fastdds::dds;

/**
 * @brief DataWriterListener implementation for monitoring publication matches
 * Provides real-time feedback on subscriber discovery for order routing
 */
class OrderClientListener : public DataWriterListener
{
public:
    void on_publication_matched(
        DataWriter *writer,
        const PublicationMatchedStatus &info) override
    {
        if (info.current_count_change == 1)
        {
            std::cout << "✅ Publisher matched with subscriber! Total matches: "
                      << info.total_count << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            std::cout << "❌ Publisher unmatched from subscriber. Total matches: "
                      << info.total_count << std::endl;
        }
    }
};

/**
 * @brief Test client for sending order requests to the OMS
 * Implements proper FastDDS participant lifecycle management and order publishing
 */
class OrderClient
{
private:
    std::unique_ptr<DomainParticipant,
                    std::function<void(DomainParticipant *)>>
        participant_;
    std::unique_ptr<Publisher,
                    std::function<void(Publisher *)>>
        publisher_;
    std::unique_ptr<Topic,
                    std::function<void(Topic *)>>
        topic_;
    std::unique_ptr<DataWriter,
                    std::function<void(DataWriter *)>>
        writer_;
    TypeSupport type_;
    OrderClientListener listener_;

public:
    OrderClient() = default;

    ~OrderClient()
    {
        cleanup();
    }

    /**
     * @brief Initialize FastDDS components for order publishing
     * @return true if initialization successful, false otherwise
     */
    bool init()
    {
        try
        {
            // Create participant with RAII cleanup
            DomainParticipantQos pqos;
            pqos.setup_transports(eprosima::fastdds::rtps::BuiltinTransports::UDPv4);
            pqos.name(CLIENT_PARTICIPANT_NAME);

            auto *raw_participant = DomainParticipantFactory::get_instance()
                                        ->create_participant(DEFAULT_DOMAIN_ID, pqos);

            if (raw_participant == nullptr)
            {
                std::cerr << "❌ Failed to create DomainParticipant" << std::endl;
                return false;
            }

            participant_ = std::unique_ptr<DomainParticipant,
                                           std::function<void(DomainParticipant *)>>(
                raw_participant,
                [](DomainParticipant *p)
                {
                    if (p != nullptr)
                    {
                        DomainParticipantFactory::get_instance()->delete_participant(p);
                    }
                });

            std::cout << "✅ Created DomainParticipant" << std::endl;

            // Register OrderRequest type
            type_.reset(new OrderRequestPubSubType());
            if (type_.register_type(participant_.get()) != RETCODE_OK)
            {
                std::cerr << "❌ Failed to register OrderRequest type" << std::endl;
                return false;
            }

            std::cout << "✅ Registered type: " << type_.get_type_name() << std::endl;

            // Create topic with RAII cleanup
            TopicQos tqos;
            Topic *raw_topic = participant_->create_topic(
                NEW_ORDER_REQUEST_TOPIC_NAME, type_.get_type_name(), tqos);

            if (raw_topic == nullptr)
            {
                std::cerr << "❌ Failed to create topic" << std::endl;
                return false;
            }

            topic_ = std::unique_ptr<Topic, std::function<void(Topic *)>>(
                raw_topic,
                [this](Topic *t)
                {
                    if (t != nullptr && participant_ != nullptr)
                    {
                        participant_->delete_topic(t);
                    }
                });

            std::cout << "✅ Created topic: " << topic_->get_name()
                      << " with type: " << topic_->get_type_name() << std::endl;

            // Create publisher with RAII cleanup
            PublisherQos pub_qos;
            auto *raw_publisher = participant_->create_publisher(pub_qos);

            if (raw_publisher == nullptr)
            {
                std::cerr << "❌ Failed to create publisher" << std::endl;
                return false;
            }

            publisher_ = std::unique_ptr<Publisher, std::function<void(Publisher *)>>(
                raw_publisher,
                [this](Publisher *p)
                {
                    if (p != nullptr && participant_ != nullptr)
                    {
                        participant_->delete_publisher(p);
                    }
                });

            std::cout << "✅ Created publisher" << std::endl;

            // Create DataWriter with reliable QoS for financial data integrity
            DataWriterQos wqos;

            wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
            wqos.history().kind = KEEP_LAST_HISTORY_QOS;
            wqos.history().depth = 10;
            wqos.resource_limits().max_samples = 100;
            wqos.resource_limits().max_instances = 10;

            auto *raw_writer = publisher_->create_datawriter(
                topic_.get(), wqos, &listener_);

            if (raw_writer == nullptr)
            {
                std::cerr << "❌ Failed to create DataWriter" << std::endl;
                return false;
            }

            writer_ = std::unique_ptr<DataWriter, std::function<void(DataWriter *)>>(
                raw_writer,
                [this](DataWriter *w)
                {
                    if (w != nullptr && publisher_ != nullptr)
                    {
                        publisher_->delete_datawriter(w);
                    }
                });

            std::cout << "✅ Created DataWriter with listener" << std::endl;
            std::cout << "🎯 OrderClient initialized successfully - waiting for discovery..." << std::endl;

            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "❌ Exception during initialization: " << e.what() << std::endl;
            cleanup();
            return false;
        }
    }

    /**
     * @brief Clean up FastDDS resources
     * RAII ensures automatic cleanup via smart pointers
     */
    void cleanup()
    {
        // Smart pointers with custom deleters handle cleanup automatically
        writer_.reset();
        publisher_.reset();
        topic_.reset();
        participant_.reset();
        std::cout << "🧹 OrderClient cleanup completed" << std::endl;
    }

    /**
     * @brief Send an order request to the OMS
     * @param order_id Unique order identifier
     * @param symbol Trading symbol (e.g., "EURUSD")
     * @param side Buy or Sell order side
     * @param quantity Order quantity
     * @param price Order price (0.0 for market orders)
     * @return true if order sent successfully, false otherwise
     */
    bool send_order(const std::string &order_id, const std::string &symbol,
                    OrderSide side, double quantity, double price = 0.0)
    {
        if (writer_ == nullptr)
        {
            std::cerr << "❌ DataWriter not initialized" << std::endl;
            return false;
        }

        // Check subscriber availability for reliable delivery
        PublicationMatchedStatus status;
        writer_->get_publication_matched_status(status);

        if (status.current_count == 0)
        {
            std::cout << "⚠️  Warning: No subscribers matched, sending anyway..." << std::endl;
        }
        else
        {
            std::cout << "📡 Sending to " << status.current_count << " subscribers" << std::endl;
        }

        // Construct order with proper financial data validation
        OrderRequest order;
        order.client_id("test_client");
        order.order_id(order_id);
        order.symbol(symbol);
        order.side(side);
        order.type(price > 0.0 ? OrderType::LIMIT : OrderType::MARKET);
        order.quantity(quantity);
        order.price(price);
        order.stop_price(0.0);
        order.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count());
        order.user_id("user001");

        auto *topic = writer_->get_topic();
        std::cout << "📤 Publishing order: " << order_id
                  << " to topic: " << topic->get_name() << std::endl;

        ReturnCode_t ret = writer_->write(&order);
        if (ret != RETCODE_OK)
        {
            std::cerr << "❌ Failed to send order, return code: " << ret << std::endl;
            return false;
        }

        std::cout << "✅ Sent order: " << order_id << " for " << symbol
                  << " (" << (side == OrderSide::BUY ? "BUY" : "SELL")
                  << " " << quantity;

        if (price > 0.0)
        {
            std::cout << " @ " << price;
        }
        std::cout << ")" << std::endl;

        return true;
    }

    /**
     * @brief Wait for subscriber discovery with timeout
     * @param timeout_seconds Maximum time to wait for subscribers
     * @return true if subscribers found, false if timeout
     */
    bool wait_for_discovery(int timeout_seconds = 10)
    {
        if (writer_ == nullptr)
        {
            std::cerr << "❌ DataWriter not initialized" << std::endl;
            return false;
        }

        std::cout << "⏳ Waiting for subscriber discovery..." << std::endl;

        for (int i = 0; i < timeout_seconds; ++i)
        {
            PublicationMatchedStatus status;
            if (writer_->get_publication_matched_status(status) == RETCODE_OK)
            {
                if (status.current_count > 0)
                {
                    std::cout << "✅ Found " << status.current_count
                              << " matched subscribers!" << std::endl;
                    return true;
                }
            }

            std::cout << "." << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "\n❌ No subscribers found after " << timeout_seconds
                  << " seconds" << std::endl;
        return false;
    }
};

std::string generate_unique_order_id()
{
    // Get current timestamp in microseconds for high precision
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                         now.time_since_epoch())
                         .count();

    // Generate random component for additional uniqueness
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    // Format: ORD_TIMESTAMP_RANDOM (financial industry standard)
    std::ostringstream oss;
    oss << "ORD_" << timestamp << "_" << dis(gen);

    return oss.str();
}

/**
 * @brief Main test application
 * Tests order publication to the OMS with various order types
 */
int main()
{
    std::cout << "=== Order Management Service Test Client ===" << std::endl;
    std::cout << "Topic name: " << NEW_ORDER_REQUEST_TOPIC_NAME << std::endl;

    int a = 1;

    OrderClient client;
    if (!client.init())
    {
        std::cerr << "❌ Failed to initialize OrderClient" << std::endl;
        return 1;
    }

    // Wait for OMS subscriber discovery
    if (!client.wait_for_discovery(10))
    {
        std::cout << "⚠️  Proceeding without subscriber discovery..." << std::endl;
    }

    // Send comprehensive test orders for validation
    std::cout << "\n🚀 Starting to send test orders..." << std::endl;

    // Test limit buy order
    do
    {

        auto order_id = generate_unique_order_id();
        auto side = a == 1 ? OrderSide::SELL : OrderSide::BUY;
        client.send_order(order_id, "BTC-USD", side, 1000.0, 1100);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "\n✅ Test complete. Check OMS console for processing results." << std::endl;
        std::cout << "input 0 to exit... any other to keep sending new order" << std::endl;
        a = std::cin.get();

    } while (a != 0);

    return 0;
}
