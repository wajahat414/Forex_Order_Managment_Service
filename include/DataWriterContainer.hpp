
#pragma once

#include <BasicDomainParticipant.h>
#include <Common.h>

#include <NewOrderSinglePubSubTypes.hpp>

using namespace DistributedATS_NewOrderSingle;
namespace OrderManagmentService
{
    struct DataWriterContainer
    {

        distributed_ats_utils::topic_tuple_ptr<NewOrderSingle> _new_order_single_tuple;
        distributed_ats_utils::data_writer_ptr _new_order_single_dw;

        DataWriterContainer(distributed_ats_utils::basic_domain_participant_ptr participant_ptr)
        {
            _new_order_single_tuple = participant_ptr->make_topic<NewOrderSinglePubSubType, NewOrderSingle>(NEW_ORDER_SINGLE_TOPIC_NAME);

            _new_order_single_dw = participant_ptr->make_data_writer(_new_order_single_tuple);
        }
    };

    using DataWriterContainerPtr = std::shared_ptr<OrderManagmentService::DataWriterContainer>;

}; // namespace OrderManagementService
