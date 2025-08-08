#pragma once

#include <thread>
#include "ConcurrentQueue.h"
#include "Application.hpp"

#pragma once

#include <thread>
#include "ConcurrentQueue.h"
#include "Application.hpp"

namespace OrderManagmentService
{

    template <class TT>
    class message_composer_thread
    {
    public:
        message_composer_thread(OrderManagmentService::Application &application, std::function<void(OrderManagmentService::Application &, TT &)> &&processor, const std::string &name,
                                unsigned long wait_timeout_us = 1000) : _processor(processor), _name(name), _wait_timeout_us(wait_timeout_us)
        {

            LOG4CXX_INFO(logger, "Starting processor : [" << _name << "]");

            _done.store(false);
            _fix_publisher_thread = std::thread([&]()
                                                {
                                                    while (!_done.load())
                                                    {
                                                        TT dds_message;
                                                        while (_dds_msg_queue.try_pop(dds_message))
                                                        {
                                                            LOG4CXX_INFO(logger, "Processing: [" << _name << "]");
                                                            _processor(application, dds_message);
                                                            LOG4CXX_INFO(logger, "Processed: [" << _name << "]");
                                                        }

                                                        std::this_thread::sleep_for(std::chrono::microseconds(_wait_timeout_us));
                                                    }

                                                    LOG4CXX_INFO(logger, "Exiting processes: [" << _name << "]"); });
        };

        void enqueue_dds_message(const TT &dds_msg)
        {
            _dds_msg_queue.push(dds_msg);
        }

        virtual ~message_composer_thread()
        {
            LOG4CXX_INFO(logger, "Stopping processor : [" << _name << "]");

            _done.store(true);

            _fix_publisher_thread.join();

            LOG4CXX_INFO(logger, "Stopped processor");
        };

    private:
        concurrent_queue<TT> _dds_msg_queue;

        std::thread _fix_publisher_thread;
        std::atomic<bool> _done{false};

        unsigned long _wait_timeout_us;

        std::string _name;

        std::function<void(OrderManagmentService::Application &, TT &)> _processor;
    };

}
