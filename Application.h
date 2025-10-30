//
// Created by Vaasu Bisht on 24/10/25.
//

#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#include "Scheduler/OrderBookScheduler.h"
#include "Scheduler/OrderInjectorScheduler.h"
#include "Config/ConfigReader.h"
#include "ThreadAssignmentManager.h"
#include "LiveMetricsProducer.h"

/**
 * @class Application
 * @brief The main application class responsible for initializing and managing all
 * core components, including the OrderBookScheduler.
 */
class Application {
 ConfigReader::Config mConfig;
 std::shared_ptr<OrderBookScheduler> mOrderBookScheduler;
 std::shared_ptr<OrderInjectorScheduler> mOrderInjectorScheduler;
 std::shared_ptr<ThreadAssignmentManager> mAssignmentManager;

public:

 /** @brief Constructor */
 explicit Application(ConfigReader::Config config) : mConfig(std::move(config)) {}

 /**
  *@brief Initializes and starts all schedulers and worker threads.
  */
 void start();

 /**
  * @brief Gracefully stops all schedulers and joins their worker threads.
  */
 void shutdown();


 void simulate();
};



#endif //APPLICATION_H
