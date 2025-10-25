//
// Created by Vaasu Bisht on 21/10/25.
//

#pragma once

#ifndef ORDERBOOKSCHEDULER_H
#define ORDERBOOKSCHEDULER_H

#include "Scheduler.h"
#include "../OrderBook/OrderBook.h"
#include <iostream>
/**
 * @class OrderBookScheduler
 * @extends Scheduler
 */

class OrderBookScheduler final : public Scheduler {
public:
 using SymbolToWorkerMap = std::unordered_map<Symbol, std::string>;
private:

 SymbolToWorkerMap mSymbolToWorkerMap;
 std::string mPrefix;
 size_t mWorkersCnt;
 mutable std::shared_mutex mObsLock; ///< Mutex for SymbolToWorkerMap

 /**
  * @brief Get worker from symbol.
  * @throws std::runtime_error when there is no worker id for given symbol.
  */
 Worker::Id getWorker(const Symbol& s) const
 {
  std::shared_lock<std::shared_mutex> rlk(mObsLock); // read-only lock
  const auto it = mSymbolToWorkerMap.find(s);
  if(it == mSymbolToWorkerMap.end())
  {
   throw std::runtime_error("No worker mapping for "+ s);
  }
  return it->second;
 }

public:

 /**
  * @brief Constructor.
  * Initializes the threads.
  */
 OrderBookScheduler(std::string  workerPrefix, const size_t cnt,
                    SymbolToWorkerMap symbolToWorkerMap):
 mSymbolToWorkerMap(std::move(symbolToWorkerMap)), mPrefix(std::move(workerPrefix)),
 mWorkersCnt(cnt)
 {
  createWorkers(mPrefix,mWorkersCnt);
 }

 void processOrder(OrderPtr order);
};



#endif //ORDERBOOKSCHEDULER_H
