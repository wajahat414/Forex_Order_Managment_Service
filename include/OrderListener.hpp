#pragma once

#include <string>
#include <memory>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

class RiskValidator;
class OrderRouter;

class OrderListener : public DataReaderListener
{
public:
    OrderListener(std::shared_ptr<RiskValidator> risk_validator,
                  std::shared_ptr<OrderRouter> order_router);
    ~OrderListener() override;

    void on_data_available(DataReader *reader) override;
    void on_subscription_matched(DataReader *reader, const SubscriptionMatchedStatus &info) override;

    bool init();
    void cleanup();

private:
    DomainParticipant *participant_;
    Subscriber *subscriber_;
    Topic *topic_;
    DataReader *reader_;
    TypeSupport type_;

    std::shared_ptr<RiskValidator> risk_validator_;
    std::shared_ptr<OrderRouter> order_router_;

    void process_order(const OrderRequest &order);
};
