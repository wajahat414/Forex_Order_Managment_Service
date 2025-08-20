#pragma once

#include "Adapter.hpp"
#include "Application.hpp"
#include "DataWriterContainer.hpp"
#include "NewOrderSingle.hpp"
#include "OrderMessage.hpp"
#include "OrderMessagePubSubTypes.hpp"
#include <Common.h>
#include <memory>
#include <string>

using namespace eprosima::fastdds::dds;

class OrderRouter {
public:
  OrderRouter(OrderManagmentService::Application &application);
  ~OrderRouter();

  bool initialize();
  void cleanup();

  bool route_to_matching_engine(const OrderRequest &order);

private:
  OrderManagmentService::Application &application;
};
