//
// Created by Vaasu Bisht on 25/10/25.
//

#pragma once

#ifndef ORDERINJECTORSCHEDULER_H
#define ORDERINJECTORSCHEDULER_H

#include "OrderBookScheduler.h"

/**
 * @class OrderInjectorScheduler
 * @brief A scheduler that creates Order objects from incoming data (IPC) and delegates order processing
 * to an OrderBookScheduler.
 *
 * These workers will be listening to IPC to make construct order objects and call specific delegate to
 * order book schedulers.
 */
class OrderInjectorScheduler  final : public Scheduler {
 std::string mWorkerPrefix;
 size_t mWorkerCount;
 std::shared_ptr<OrderBookScheduler> mOrderBookScheduler;

 // For round-robin assignment to injector workers
 mutable std::atomic<size_t> mNextWorkerId{0};

 /**
  * @brief Get the worker id responsible for handling the order.
  * It will be round-robin implementation.
  * @return Worker ID.
  */
 Worker::Id getWorkerIdForOrder() const;

public:
 /** @brief Constructor. Initializes workers */
 OrderInjectorScheduler(std::string workerPrefix, const size_t count,
                       std::shared_ptr<OrderBookScheduler> obs)
     : mWorkerPrefix(std::move(workerPrefix)),
       mWorkerCount(count),
       mOrderBookScheduler(std::move(obs))
 {
  createWorkers(mWorkerPrefix, mWorkerCount);
 }

 /**
  * @brief Process a raw incoming message (string from IPC). Converts it to an Order object
  * and delegates to OrderBookScheduler.
  * @param orderMessage Raw order data as string
  */
 void processIncomingOrder(const std::string& orderMessage);
};



#endif //ORDERINJECTORSCHEDULER_H
