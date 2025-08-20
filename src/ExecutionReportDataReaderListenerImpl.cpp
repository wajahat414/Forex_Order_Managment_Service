#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>

#include "Application.hpp"
#include "ExecutionReportDataReaderListenerImpl.h"
#include <ExecutionReport.hpp>
#include <ExecutionReportLogger.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>

auto const exec_report_processor =
    [](OrderManagmentService::Application &application,
       DistributedATS_ExecutionReport::ExecutionReport &execution_report) {
      execution_report.fix_header().BeginString("FIX4.4");
      execution_report.fix_header().TargetCompID(
          execution_report.DATS_Destination());
      execution_report.fix_header().SenderCompID(
          execution_report.DATS_DestinationUser());
      execution_report.fix_header().SendingTime(0);

      std::stringstream ss;
      ExecutionReportLogger::log(ss, execution_report);

      LOG4CXX_INFO(logger, "Execution Report " << ss.str());
      bool response = application.onExecutionReportRecieved(execution_report);
      if (response) {
        LOG4CXX_INFO(logger, "OrderResponse Report Published Successfully"
                                 << execution_report.OrderID());
      } else {
        LOG4CXX_ERROR(logger, "OrderResponse Failed to publish report for order"
                                  << execution_report.OrderID());
      }
    };

namespace OrderManagmentService {
ExecutionReportDataReaderListenerImpl::ExecutionReportDataReaderListenerImpl(
    OrderManagmentService::Application &application)
    : _processor(application, exec_report_processor,
                 "ExecutionReportDataReaderListenerImpl", 1) {}

void ExecutionReportDataReaderListenerImpl::on_data_available(
    eprosima::fastdds::dds::DataReader *reader) {

  DistributedATS_ExecutionReport::ExecutionReport executionReport;
  eprosima::fastdds::dds::SampleInfo info;

  if (reader->take_next_sample(&executionReport, &info) ==
      eprosima::fastdds::dds::RETCODE_OK) {
    if (info.valid_data) {
      std::cout << "Execution Report Received" << std::endl;
      _processor.enqueue_dds_message(executionReport);
    }
  }
}
} // namespace OrderManagmentService
