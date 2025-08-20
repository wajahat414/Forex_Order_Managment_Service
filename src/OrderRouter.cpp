#include "OrderRouter.hpp"
#include <BasicDomainParticipant.h>
#include <iostream>
OrderRouter::OrderRouter(OrderManagmentService::Application &application)
    : application(application) {}

OrderRouter::~OrderRouter() {}

bool OrderRouter::route_to_matching_engine(const OrderRequest &order) {

  return application.onOrderRequestMessage(order);
}
