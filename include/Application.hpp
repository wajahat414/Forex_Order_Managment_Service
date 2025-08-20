#pragma once
#include "DataWriterContainer.hpp"
#include <ExecutionReport.hpp>
#include <OrderMessage.hpp>
#include <log4cxx/logger.h>
namespace OrderManagmentService {

class Application {

private:
  DataWriterContainerPtr _dataWriterContainer;
  std::string _dataService;
  std::string _senderCompId;
  const std::string _target = "MATCHING_ENGINE";

public:
  const std::string &sender_name() const { return _senderCompId; };
  const std::shared_ptr<OrderManagmentService::DataWriterContainer> &
  get_data_writer_container() const {
    return _dataWriterContainer;
  }
  bool onOrderRequestMessage(const OrderRequest &order_request);
  bool onExecutionReportRecieved(
      const DistributedATS_ExecutionReport::ExecutionReport &executionReport);

  Application(std::string dataService, std::string senderCompId,
              DataWriterContainerPtr dataWriterContainer)
      : _dataService(dataService), _senderCompId(senderCompId),
        _dataWriterContainer(dataWriterContainer) {};
};

} // namespace OrderManagmentService
