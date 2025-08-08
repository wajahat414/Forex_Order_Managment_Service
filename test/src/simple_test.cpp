// test_new_order.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

// Include the generated types - adjust paths as needed
#include "NewOrderSinglePubSubTypes.hpp"
#include "NewOrderSingle.hpp"

using namespace eprosima::fastdds::dds;

class TestListener : public DataWriterListener
{
public:
    void on_publication_matched(
        DataWriter *writer,
        const PublicationMatchedStatus &info) override
    {
        std::cout << "*** PUBLICATION MATCHED! Readers: " << info.current_count << " ***" << std::endl;
        matched_ = (info.current_count > 0);
    }

    bool matched_ = false;
};

int main()
{
    std::cout << "Starting NewOrderSingle Test..." << std::endl;

    // Create participant
    DomainParticipant *participant = DomainParticipantFactory::get_instance()->create_participant(
        0, PARTICIPANT_QOS_DEFAULT);

    if (participant == nullptr)
    {
        std::cerr << "Failed to create participant!" << std::endl;
        return -1;
    }
    std::cout << "✅ Participant created successfully" << std::endl;

    // Register type - FIXED VERSION
    TypeSupport type_support(new NewOrderSinglePubSubType());

    // Get the type name from the TypeSupport object
    std::string type_name = type_support.get_type_name();
    std::cout << "Type name: " << type_name << std::endl;

    // Register the type
    if (type_support.register_type(participant) != eprosima::fastdds::dds::RETCODE_OK)
    {
        std::cerr << "Failed to register type!" << std::endl;
        return -1;
    }
    std::cout << "✅ Type registered successfully" << std::endl;

    // Create topic using the registered type name
    Topic *topic = participant->create_topic(
        "NEW_ORDER_SINGLE", // Topic name
        type_name,          // Use the registered type name
        TOPIC_QOS_DEFAULT);

    if (topic == nullptr)
    {
        std::cerr << "Failed to create topic!" << std::endl;
        return -1;
    }
    std::cout << "✅ Topic created successfully" << std::endl;

    // Create publisher
    Publisher *publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (publisher == nullptr)
    {
        std::cerr << "Failed to create publisher!" << std::endl;
        return -1;
    }
    std::cout << "✅ Publisher created successfully" << std::endl;

    // Create data writer with listener
    TestListener listener;
    DataWriter *writer = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, &listener);
    if (writer == nullptr)
    {
        std::cerr << "Failed to create data writer!" << std::endl;
        return -1;
    }
    std::cout << "✅ DataWriter created successfully" << std::endl;

    std::cout << "\nWaiting for MatchingEngine to be discovered..." << std::endl;

    // Wait for discovery
    int wait_count = 0;
    while (!listener.matched_ && wait_count < 30)
    { // Wait up to 30 seconds
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Waiting... (" << ++wait_count << "/30)" << std::endl;
    }

    if (!listener.matched_)
    {
        std::cerr << "⚠️  No subscribers found, but sending message anyway..." << std::endl;
        // Continue anyway - maybe MatchingEngine will receive it
    }
    else
    {
        std::cout << "✅ MatchingEngine discovered! Sending test order..." << std::endl;
    }

    // Create and populate the test message
    NewOrderSingle testOrder;

    // Set routing fields (CRITICAL for MatchingEngine filter)
    testOrder.DATS_Source("TEST_CLIENT");
    testOrder.DATS_Destination("MATCHING_ENGINE"); // Filter requirement
    testOrder.DATS_SourceUser("TEST_USER");
    testOrder.DATS_DestinationUser("DATA_SERVICE_A"); // Must match DataService name

    // Set order fields
    testOrder.ClOrdID("TEST_ORDER_001");
    testOrder.Symbol("BTC-USD");
    testOrder.SecurityExchange("BTC_MARKET"); // Filter requirement
    testOrder.Side(1);                        // Buy
    testOrder.OrderQty(10);
    testOrder.Price(10050);
    testOrder.OrdType(2);     // Limit order
    testOrder.TimeInForce(1); // GTC

    // Set FIX header fields
    testOrder.fix_header().BeginString("FIX.4.4");
    testOrder.fix_header().MsgType("D");
    testOrder.fix_header().SenderCompID("TEST_CLIENT");
    testOrder.fix_header().TargetCompID("MATCHING_ENGINE");
    testOrder.fix_header().SendingTime(std::chrono::duration_cast<std::chrono::microseconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count());

    // Print the message we're sending
    std::cout << "\n=== Sending Test Message ===" << std::endl;
    std::cout << "DATS_Source: " << testOrder.DATS_Source() << std::endl;
    std::cout << "DATS_Destination: " << testOrder.DATS_Destination() << std::endl;
    std::cout << "DATS_SourceUser: " << testOrder.DATS_SourceUser() << std::endl;
    std::cout << "DATS_DestinationUser: " << testOrder.DATS_DestinationUser() << std::endl;
    std::cout << "SecurityExchange: " << testOrder.SecurityExchange() << std::endl;
    std::cout << "Symbol: " << testOrder.Symbol() << std::endl;
    std::cout << "ClOrdID: " << testOrder.ClOrdID() << std::endl;
    std::cout << "Side: " << testOrder.Side() << std::endl;
    std::cout << "Qty: " << testOrder.OrderQty() << std::endl;
    std::cout << "Price: " << testOrder.Price() << std::endl;

    // Send the message
    ReturnCode_t result = writer->write(&testOrder);
    if (result == eprosima::fastdds::dds::RETCODE_OK)
    {
        std::cout << "\n✅ Message sent successfully!" << std::endl;
    }
    else
    {
        std::cout << "\n❌ Failed to send message! Error code: " << (int)result << std::endl;
        return -1;
    }

    std::cout << "\n🔍 Check MatchingEngine logs for 'Kuddos New Order Recieved'" << std::endl;
    std::cout << "Expected filter match: DATS_Destination='MATCHING_ENGINE' AND SecurityExchange='BTC_MARKET'" << std::endl;
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();

    // Cleanup
    participant->delete_contained_entities();
    DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}
