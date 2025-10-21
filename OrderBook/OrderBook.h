//
// Created by Vaasu Bisht on 21/10/25.
//
#pragma once
#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <sstream>

/**
 * @class OrderBook
 * @brief Manages buy and sell orders of a specific stock symbol.
 * The architecture design decision is to keep one order book per stock symbol.
 * Each order book executes exclusively on a single dedicated thread, ensuring
 * that only one thread interacts with its data structures at any given time.
 *
 * @details
 *
 * This thread confinement model eliminates the need for internal locking within
 * the OrderBook, greatly simplifying concurrency management and improving performance.
 * The scheduler ensures that all operations related to a specific symbol are always
 * routed to the same worker thread.
 *
 * There can be scenarios where:
 * - A thread handles exactly one order book (for high-volume stocks).
 * - A thread handles multiple order books (for low-volume stocks).
 *
 * Keeping one order book per symbol makes sense because:
 * 1. Stocks are independent: Orders of one stock do not affect another.
 * 2. Performance isolation: Heavy trading in one symbol doesn’t slow down others.
 * 3. Simpler concurrency: Since only one thread accesses an order book, no mutexes or atomic
 *    synchronization are needed inside the book for matching, insertion, or updates.
 * 4. Easier fault containment: A crash or circuit breaker in one symbol affects only that book.
 * 5. Circuit breakers per stock: Trading halts and protections can be applied individually.
 */
class OrderBook {
    /**
     * @struct Stats
     * @brief Structure for tracking statistics of order book.
     */
    struct Stats
    {
        uint64_t totalOrdersCancelled{0};
        uint64_t totalOrdersAdded{0};
        uint64_t totalOrdersFulfilled{0};
        uint64_t totalVolume{0};
        uint64_t totalTrades{0};

        /**
         * @brief Reset all statistics counters to zero.
         * This is atomic and thread-safe for each counter individually.
         */
        void reset()
        {
            totalOrdersAdded = 0;
            totalOrdersCancelled = 0;
            totalOrdersFulfilled = 0;
            totalVolume = 0;
            totalTrades = 0;
        }

        /**
         * @brief Generate a human-readable string representation of all stats.
         * Useful for logging or debugging.
         */
        std::string toString() const
        {
            std::ostringstream oss;
            oss << "Stats { "
                << "totalOrdersAdded=" << totalOrdersAdded
                << ", totalOrdersCancelled=" << totalOrdersCancelled
                << ", totalOrdersFulfilled=" << totalOrdersFulfilled
                << ", totalVolume=" << totalVolume
                << ", totalTrades=" << totalTrades
                << " }";
            return oss.str();
        }
    };
};



#endif //ORDERBOOK_H
