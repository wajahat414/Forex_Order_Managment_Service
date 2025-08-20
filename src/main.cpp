#include "DataReaderContainer.h"
#include "OrderManagementService.hpp"
#include <Application.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <iostream>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <signal.h>

std::unique_ptr<OrderManagementService> g_oms_service;

void signal_handler(int signal) {
  std::cout << "\nReceived signal " << signal << ". Shutting down..."
            << std::endl;
  if (g_oms_service) {
    g_oms_service->stop();
  }
}

int main(int argc, char *argv[]) {

  try {
    log4cxx::xml::DOMConfigurator::configure("../config/log4cxx.xml");
  } catch (...) {
    // Fallback to basic configuration if XML file not found
    log4cxx::BasicConfigurator::configure();
    std::cout << "Using basic log4cxx configuration" << std::endl;
  }
  std::string sender_comp_id = "OrderManagmentService";
  std::string data_service_name = "DATA_SERVICE_A";
  auto participant_ptr =
      std::make_shared<distributed_ats_utils::basic_domain_participant>(
          0, sender_comp_id);
  participant_ptr->create_publisher();
  participant_ptr->create_subscriber();
  LOG4CXX_INFO(logger, "SenderCompID : [" << sender_comp_id
                                          << "] | DataService : ["
                                          << data_service_name << "]");

  auto data_writer_container =
      std::make_shared<OrderManagmentService::DataWriterContainer>(
          participant_ptr);

  OrderManagmentService::Application application(
      data_service_name, sender_comp_id, data_writer_container);

  auto data_reader_container =
      std::make_shared<OrderManagmentService::DataReaderContainer>(
          participant_ptr, application, sender_comp_id);

  std::cout << "=== Order Management Service ===" << std::endl;

  // Configure FastDDS logging to reduce noise
  eprosima::fastdds::dds::Log::SetVerbosity(
      eprosima::fastdds::dds::Log::Warning);
  eprosima::fastdds::dds::Log::SetCategoryFilter(std::regex("SYSTEM"));

  // Load FastDDS XML profile if it exists
  std::string profile_file = "config/fastdds_profile.xml";
  if (argc >= 4) {
    profile_file = argv[3];
  }

  // Try to load XML profile (ignore if file doesn't exist)

  // Set up signal handlers
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Default configuration files
  std::string user_config = "config/users.json";
  std::string symbol_config = "config/symbols.json";

  // Parse command line arguments
  if (argc >= 2) {
    user_config = argv[1];
  }
  if (argc >= 3) {
    symbol_config = argv[2];
  }

  std::cout << "Using user config: " << user_config << std::endl;
  std::cout << "Using symbol config: " << symbol_config << std::endl;
  std::cout << "Using FastDDS profile: " << profile_file << " (optional)"
            << std::endl;

  // Create and initialize the service
  g_oms_service = std::make_unique<OrderManagementService>();

  if (!g_oms_service->initialize(user_config, symbol_config, application)) {
    std::cerr << "Failed to initialize Order Management Service" << std::endl;
    return 1;
  }

  // Start the service
  g_oms_service->start();

  std::cout << "Order Management Service shutdown complete" << std::endl;
  return 0;
}
