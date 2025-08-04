#include "OrderManagementService.hpp"
#include <iostream>
#include <signal.h>

std::unique_ptr<OrderManagementService> g_oms_service;

void signal_handler(int signal)
{
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    if (g_oms_service)
    {
        g_oms_service->stop();
    }
}

int main(int argc, char *argv[])
{
    std::cout << "=== Order Management Service ===" << std::endl;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Default configuration files
    std::string user_config = "config/users.json";
    std::string symbol_config = "config/symbols.json";

    // Parse command line arguments
    if (argc >= 2)
    {
        user_config = argv[1];
    }
    if (argc >= 3)
    {
        symbol_config = argv[2];
    }

    std::cout << "Using user config: " << user_config << std::endl;
    std::cout << "Using symbol config: " << symbol_config << std::endl;

    // Create and initialize the service
    g_oms_service = std::make_unique<OrderManagementService>();

    if (!g_oms_service->initialize(user_config, symbol_config))
    {
        std::cerr << "Failed to initialize Order Management Service" << std::endl;
        return 1;
    }

    // Start the service
    g_oms_service->start();

    std::cout << "Order Management Service shutdown complete" << std::endl;
    return 0;
}
