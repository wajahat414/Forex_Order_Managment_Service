#pragma once
#include "DataWriterContainer.hpp"
#include <log4cxx/logger.h>
#include <OrderMessage.hpp>
namespace OrderManagmentService
{

    class Application
    {

    private:
        DataWriterContainerPtr _dataWriterContainer;
        std::string _dataService;
        std::string _senderCompId;
        const std::string _target = "MATCHING_ENGINE";

    public:
        const std::string &sender_name() const { return _senderCompId; };
        const std::shared_ptr<OrderManagmentService::DataWriterContainer> &get_data_writer_container() const
        {
            return _dataWriterContainer;
        }
        bool onOrderRequestMessage(const OrderRequest &order_request);

        Application(std::string dataService, std::string senderCompId, DataWriterContainerPtr dataWriterContainer) : _dataService(dataService), _senderCompId(senderCompId), _dataWriterContainer(dataWriterContainer) {};
    };

}
