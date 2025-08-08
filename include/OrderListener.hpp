#pragma once

#include <string>
#include <memory>

#include <BasicDomainParticipant.h>
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"
#include <Common.h>

class RiskValidator;
class OrderRouter;

class OrderListener : public eprosima::fastdds::dds::DataReaderListener
{
public:
    OrderListener(std::shared_ptr<RiskValidator> risk_validator,
                  std::shared_ptr<OrderRouter> order_router);
    ~OrderListener() override;

    void on_data_available(eprosima::fastdds::dds::DataReader *reader) override;
    void on_subscription_matched(eprosima::fastdds::dds::DataReader *reader, const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) override;
    bool init();
    void cleanup();

private:
    distributed_ats_utils::basic_domain_participant_ptr participant_;
    distributed_ats_utils::topic_tuple_ptr<OrderRequestPubSubType> topic_tuple_;
    distributed_ats_utils::data_reader_tuple_ptr<OrderRequestPubSubType> reader_tuple_;

    std::shared_ptr<RiskValidator> risk_validator_;
    std::shared_ptr<OrderRouter> order_router_;

    void process_order(const OrderRequest &order);
};
