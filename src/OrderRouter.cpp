#include "OrderRouter.hpp"
#include <iostream>
#include <BasicDomainParticipant.h>
OrderRouter::OrderRouter(OrderManagmentService::Application &application)
    : application(application)
{
}

OrderRouter::~OrderRouter()
{
}

bool OrderRouter::route_to_matching_engine(const OrderRequest &order)
{

    return application.onOrderRequestMessage(order);
}
