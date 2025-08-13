#pragma once

#include <string>
#include <memory>
#include "DataWriterContainer.hpp"
#include "OrderMessage.hpp"
#include "NewOrderSingle.hpp"
#include "OrderMessagePubSubTypes.hpp"
#include <Common.h>
#include "Adapter.hpp"
#include "Application.hpp"

using namespace eprosima::fastdds::dds;

class OrderRouter
{
public:
    OrderRouter(OrderManagmentService::Application &application);
    ~OrderRouter();

    bool initialize();
    void cleanup();

    bool route_to_matching_engine(const OrderRequest &order);

private:
    OrderManagmentService::Application &application;
};
