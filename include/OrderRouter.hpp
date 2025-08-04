#pragma once

#include <string>
#include <memory>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

class OrderRouter
{
public:
    OrderRouter();
    ~OrderRouter();

    bool initialize();
    void cleanup();

    bool route_to_matching_engine(const OrderRequest &order);
    bool send_order_response(const OrderResponse &response);

private:
    DomainParticipant *participant_;
    Publisher *publisher_;
    Topic *matching_engine_topic_;
    Topic *response_topic_;
    DataWriter *matching_engine_writer_;
    DataWriter *response_writer_;
    TypeSupport order_type_;
    TypeSupport response_type_;

    bool setup_matching_engine_publisher();
    bool setup_response_publisher();
};
