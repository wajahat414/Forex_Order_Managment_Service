#include <BasicDomainParticipant.h>
#include "Application.hpp"
#include "ExecutionReport.hpp"
#include "ExecutionReportPubSubTypes.hpp"
#include "ExecutionReportDataReaderListenerImpl.h"

namespace OrderManagmentService

{
    struct DataReaderContainer
    {

        distributed_ats_utils::topic_tuple_ptr<DistributedATS_ExecutionReport::ExecutionReport> _execution_report_topic_tuple;
        distributed_ats_utils::data_reader_tuple_ptr<DistributedATS_ExecutionReport::ExecutionReport> _execution_report_data_reader_tuple;
        std::string target_comp_id_filter = "DATS_Destination=%0";
        DataReaderContainer(distributed_ats_utils::basic_domain_participant_ptr participant_ptr, Application &application, std::string name)
        {
            _execution_report_topic_tuple = participant_ptr->make_topic<DistributedATS_ExecutionReport::ExecutionReportPubSubType, DistributedATS_ExecutionReport::ExecutionReport>(EXECUTION_REPORT_TOPIC_NAME);

            // _execution_report_data_reader_tuple = participant_ptr->make_data_reader_tuple(_execution_report_topic_tuple, new OrderManagmentService::ExecutionReportDataReaderListenerImpl(application), "FILTERED_EXEC_REPORT", target_comp_id_filter, {name});
            _execution_report_data_reader_tuple = participant_ptr->make_data_reader_tuple(_execution_report_topic_tuple, new OrderManagmentService::ExecutionReportDataReaderListenerImpl(application));
        }
    };

} // namespace  OrderManagmentService
