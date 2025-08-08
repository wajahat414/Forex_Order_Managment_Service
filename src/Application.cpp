#include "Application.hpp"
#include "DataWriterContainer.hpp"
#include <OrderMessage.hpp>
#include "Adapter.hpp"
#include <NewOrderSingleLogger.hpp>

using namespace OrderManagmentService;

bool Application::onOrderRequestMessage(const OrderRequest &order_request)
{
    auto new_single_order = OrderAdapter::requestToNewOrderSingle(order_request);
    new_single_order.DATS_Source(sender_name());
    new_single_order.DATS_SourceUser(sender_name());

    new_single_order.DATS_SourceUser(_senderCompId);
    new_single_order.DATS_Destination(_target);
    new_single_order.DATS_DestinationUser("DATA_SERVICE_A");
    new_single_order.SecurityExchange("BTC_MARKET"); // Assuming FOREX as default exchange

    std::stringstream ss;
    NewOrderSingleLogger::log(ss, new_single_order);
    LOG4CXX_INFO(logger, "Publishing to DDS" << ss.str());

    auto ret = _dataWriterContainer->_new_order_single_dw->write(&new_single_order);
    if (ret != eprosima::fastdds::dds::RETCODE_OK)
    {
        LOG4CXX_ERROR(logger, "Unable to publishing to DDS:" << ss.str());
        std::cout << "Unable to to publish new Order to DDS" << ss.str();
        return false;
    }
    return true;
}
