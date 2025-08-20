
#pragma once

#include <BasicDomainParticipant.h>
#include <Common.h>

#include <NewOrderSinglePubSubTypes.hpp>
#include <OrderResponseReportPubSubTypes.hpp>

using namespace DistributedATS_NewOrderSingle;
using namespace OMS;
namespace OrderManagmentService {
struct DataWriterContainer {

  distributed_ats_utils::topic_tuple_ptr<NewOrderSingle>
      _new_order_single_tuple;
  distributed_ats_utils::topic_tuple_ptr<OrderResponseReport>
      _order_response_report_tuple; // Fixed typo and type
  distributed_ats_utils::data_writer_ptr _new_order_single_dw;
  distributed_ats_utils::data_writer_ptr _order_response_report_dw;

  DataWriterContainer(
      distributed_ats_utils::basic_domain_participant_ptr participant_ptr) {
    _new_order_single_tuple =
        participant_ptr->make_topic<NewOrderSinglePubSubType, NewOrderSingle>(
            NEW_ORDER_SINGLE_TOPIC_NAME);

    _new_order_single_dw =
        participant_ptr->make_data_writer(_new_order_single_tuple);

    _order_response_report_tuple =
        participant_ptr
            ->make_topic<OrderResponseReportPubSubType, OrderResponseReport>(
                ORDER_RESPONSE_TOPIC_NAME);

    _order_response_report_dw =
        participant_ptr->make_data_writer(_order_response_report_tuple);
  }
};

using DataWriterContainerPtr =
    std::shared_ptr<OrderManagmentService::DataWriterContainer>;

}; // namespace OrderManagmentService
