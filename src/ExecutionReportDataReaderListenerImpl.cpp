#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>

#include "Application.hpp"
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include "ExecutionReportDataReaderListenerImpl.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <ExecutionReport.hpp>

auto const exec_report_processor = [](OrderManagmentService::Application &application, ExecutionReport &execution_report)
{
    execution_report.fix_header().BeginString("FIX4.4");
    execution_report.fix_header().TargetCompID(execution_report.DATS_Destination());
    execution_report.fix_header().SenderCompID(execution_report.DATS_DestinationUser());
    execution_report.fix_header().SendingTime(0);
};

namespace OrderManagmentService
{
    ExecutionReportDataReaderListenerImpl::ExecutionReportDataReaderListenerImpl(OrderManagmentService::Application &application)
        : _processor(application, exec_report_processor, "ExecutionReportDataReaderListenerImpl", 100)
    {
    }

    void ExecutionReportDataReaderListenerImpl::on_data_available(eprosima::fastdds::dds::DataReader *reader)
    {

        ExecutionReport executionReport;
        eprosima::fastdds::dds::SampleInfo info;

        if (reader->take_next_sample(&executionReport, &info) == eprosima::fastdds::dds::RETCODE_OK)
        {
            if (info.valid_data)
            {
                std::cout << "Execution Report Received" << std::endl;
                _processor.enqueue_dds_message(executionReport);
            }
        }
    }
}
