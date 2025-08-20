#include "Application.hpp"
#include "Adapter.hpp"
#include "DataWriterContainer.hpp"
#include <NewOrderSingleLogger.hpp>
#include <OrderMessage.hpp>

using namespace OrderManagmentService;

bool Application::onOrderRequestMessage(const OrderRequest &order_request) {
  auto new_single_order = Adapter::requestToNewOrderSingle(order_request);
  new_single_order.DATS_Source(sender_name());
  new_single_order.DATS_SourceUser(sender_name());

  new_single_order.DATS_SourceUser(_senderCompId);
  new_single_order.DATS_Destination(_target);
  new_single_order.DATS_DestinationUser("DATA_SERVICE_A");
  new_single_order.SecurityExchange(
      "BTC_MARKET"); // Assuming FOREX as default exchange

  std::stringstream ss;
  NewOrderSingleLogger::log(ss, new_single_order);
  LOG4CXX_INFO(logger, "Publishing to DDS" << ss.str());

  auto ret =
      _dataWriterContainer->_new_order_single_dw->write(&new_single_order);
  if (ret != eprosima::fastdds::dds::RETCODE_OK) {
    LOG4CXX_ERROR(logger, "Unable to publishing to DDS:" << ss.str());
    std::cout << "Unable to to publish new Order to DDS" << ss.str();
    return false;
  }
  return true;
}

bool Application::onExecutionReportRecieved(
    const DistributedATS_ExecutionReport::ExecutionReport &executionReport) {
  auto response_report =
      Adapter::executionReportToOrderReponseReport(executionReport);

  auto ret =
      _dataWriterContainer->_order_response_report_dw->write(&response_report);

  if (ret != eprosima::fastdds::dds::RETCODE_OK) {

    std::cout << "Unable to to publish new Order to DDS Order Response report"
              << std::endl;
    return false;
  }
  return true;
}
