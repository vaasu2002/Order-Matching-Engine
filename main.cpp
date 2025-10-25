#include <csignal>
#include "Application.h"
#include "Config/ConfigReader.h"

std::unique_ptr<Application> gApp; // Global instance of application

void signalHandler(const int signum)
{
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down application..." << std::endl;
    if (gApp)
    {
        gApp->shutdown();
    }
    // Exit the program
    exit(signum);
}

int main()
{
    // Constructing validation chain
    const auto chain = std::make_shared<OrderValidator>();
    chain->add(std::make_unique<QuantityValidator>());
    chain->add(std::make_unique<LimitPriceRequiredValidator>());
    chain->add(std::make_unique<StopPriceRequiredValidator>());
    // Setting default configuration
    Order::SetDefaultValidator(chain);

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Initialize Application Configuration
    ConfigReader::Config config;
    try
    {
        config = ConfigReader::LoadConfig("./config.xml");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        return 1;
    }

    // Instantiate the Application
    gApp = std::make_unique<Application>(config);

    // Start the Application
    try
    {
        gApp->start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Application failed to start: " << e.what() << std::endl;
        return 1;
    }

    // Run the simulation
    gApp->simulate();


    // The application will now block in the run() method or wait for a signal.
    // Since run() is a skeleton, we'll wait for user input to simulate an ongoing process.
    std::cout << "Type 'q' and press Enter to manually stop the application." << std::endl;
    std::string input;
    while (std::cin >> input)
    {
        if (input == "q" || input == "quit")
        {
            break;
        }
    }

    // Manual shutdown if the loop is exited normally
    if (gApp)
    {
        gApp->shutdown();
    }

    return 0;
}