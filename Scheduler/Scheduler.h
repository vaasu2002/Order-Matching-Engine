//
// Created by Vaasu Bisht on 19/10/25.
//

#pragma once

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <map>

struct Worker;

/**
 * @class Scheduler
 * @brief Lightweight multithreaded task scheduler that manages multiple worker threads
 * and task queues.
 *
 * It provides a framework for async task execution across a configurable number of worker threads.
 * Each worker maintains its own task queue and continuously processes task in FIFO order.  Task can
 * be submitted to specific workers using `submitTo()` or with a returnable `std::future` via
 * `submitToWithFuture()`.
 */
class Scheduler
{
 // using WorkerMap = std::map<std::string, std::unique_ptr<Worker>>;

 // <====== Data Members ======>
 std::map<std::string, std::unique_ptr<Worker>> mWorkers; ///< Holds all the workers currently active by their id
 mutable  std::mutex mLock; ///< Mutex for WorkerMap
 bool mShutdown{false}; ///> Indicates that all workers are shutdown

public:
 /* @brief Default constructor. Initializes an empty scheduler */
 Scheduler() = default;

 /* @brief Delete copy constructor to prevent copying of v instances. */
 Scheduler(const Scheduler&) = delete;

 /* @brief Delete copy constructor to prevent copying of v instances. */
 Scheduler& operator=(const Scheduler&) = delete;

 /* @brief Destructor that ensures all workers are stopped and joined */
 // virtual ~Scheduler();

 /**
  * @brief Start all the workers
  */
 void start();

 /**
   * @brief  Creates and reserves a single worker with a unique identifier.
   * @param id The string identifier for the worker (e.g., "worker_1")
   * @throws std::runtime_error If a worker with the same id already exists.
   */
 void createWorker(const std::string& id);

 /**
  * @brief Reserves multiple workers using a naming prefix.
  * @param prefix The prefix for worker's names.
  * @param cnt The number of workers to create.
  *
  * @example
  * prefix = "workers", count = 3 -> creates 'worker_0', 'worker_1', 'worker_2'
  */
 void createWorkers(const std::string& prefix, size_t cnt);

};

#endif //SCHEDULER_H
