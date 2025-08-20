
#pragma once

#include "Application.hpp"
#include "ComposerThread.h"
#include <ExecutionReport.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
namespace OrderManagmentService {

class ExecutionReportDataReaderListenerImpl
    : public eprosima::fastdds::dds::DataReaderListener {
private:
  message_composer_thread<DistributedATS_ExecutionReport::ExecutionReport>
      _processor;

public:
  ExecutionReportDataReaderListenerImpl(Application &application);
  ~ExecutionReportDataReaderListenerImpl() override {};
  void on_data_available(eprosima::fastdds::dds::DataReader *reader) override;
};

} // namespace OrderManagmentService
