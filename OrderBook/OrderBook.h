//
// Created by Vaasu Bisht on 21/10/25.
//
#pragma once
#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <shared_mutex>
#include <string>
#include <sstream>
#include <iostream>
#include "OrderTracker/OrderTracker.h"


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
    // Shared pointer as multiple components (workers, registry, and engine) may concurrently access and
    // share the same order book instance. To ensure safe, reference-counted lifetime management decision to use
    // shared pointer was reached.
    using OrderBookPtr = std::shared_ptr<OrderBook>;
    /**
     * @struct Stats
     * @brief Structure for tracking statistics of order book.
     */
    struct Stats
    {
        // <===== Market States =====>

        uint64_t marketPrice{0}; /// < Current market price
        uint64_t lastTradePrice{0}; /// < Price at which last trade was executed.
        uint64_t lastTradQty{0}; /// < Number of shares that were traded at the last trade.

        // <===== Order Book States =====>

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
            << "marketPrice=" << marketPrice
            << ", lastTradePrice=" << lastTradePrice
            << ", lastTradQty=" << lastTradQty
            << ", totalOrdersAdded=" << totalOrdersAdded
            << ", totalOrdersCancelled=" << totalOrdersCancelled
            << ", totalOrdersFulfilled=" << totalOrdersFulfilled
            << ", totalVolume=" << totalVolume
            << ", totalTrades=" << totalTrades
            << " }";
            return oss.str();
        }
    };

    /**
     * @struct Registry
     * @brief Multiton registry: multiton mapping from Symbol -> weak_ptr<OrderBook>.
     * The Registry is thread-safe; it holds weak_ptr to avoid cycles and to
     * allow lazy creation.
     */
    struct Registry
    {
        std::unordered_map<Symbol, std::shared_ptr<OrderBook>> registry; ///> Stores OrderBook belonging to symbol.
        mutable std::shared_mutex  mtx; ///> Lock for registry data structure.

        OrderBookPtr createOrderBook(const Symbol& symbol);

        /**
         *
         * @brief Get order book if exists.
         * @return may return nullptr if weak_ptr expired
         */
        OrderBookPtr getOrderBook(const Symbol& symbol);

        /**
         *
         * @brief Get order book if exists.
         * @return may return nullptr if weak_ptr expired
         */
        OrderBookPtr getOrderBookSafe(const Symbol& symbol);

        /**
         * @param symbol
         * @return
         */
        OrderBookPtr getOrCreateOrderBook(const Symbol& symbol);

        bool exists(const Symbol& symbol) const;
        void erase(const Symbol& symbol);
        void cleanupRegistry();
        size_t size() const;
    };
    using Tracker = OrderTracker;
    using TrackerStore = std::map<Side,Tracker>;

    Symbol mSymbol;
    // Tracker mBidTracker; ///> Tracking storing BUY side order.
    // Tracker mAskTracker; ///>  Tracking storing SELL side order.
    TrackerStore mTrackerStore; ///> Stores order tracker for different side (BUY and SELL)
    Stats mStats;

    static Registry& registry()
    {
     static Registry r;
     return r;
    }

    /**
     * @brief Fetch order book of given side.
     */
    Tracker& getOrderTracker(Side side);

    /**
     * @brief Attempts to match an incoming order with orders from the opposite side.
     * @param order Incoming order being matched.
     */
    void matchOrder(OrderRawPtr order);

    /**
     * @brief Persists the order in the order book. The order is stored in the appropriate
     * price level tracker depending on its side.
     * @param order The order to be stored.
     */
    void addRestingOrder(OrderPtr order);

    /**
     * @brief Updates an order's state and determines its next action post-match.
     *
     * @details
     * Sets the order's status based on its remaining `openQty`. If the order is a
     * LIMIT order and is not fully filled, this function will add the remaining
     * quantity to the order book as a new resting order.
     *
     * @pre It must be called after the matching has been attempted.
     * @todo Implement client notifications for fills or status changes.
     */
    static void updateOrder(Order& order,Quantity remainingQty);
public:

    /** @brief Constructor */
    explicit OrderBook(Symbol symbol);

    /** @brief Destructor */

    ~OrderBook(){
      std::cout<<"Order Book destructor called"<<std::endl;
    }

    // <============== Registry accessors ==============>
    /**
     * @brief Returns the order book corresponding the given symbol. If registry does
     * not have order book it will create an order book and provide. The logic inside
     * registry is thread-safe.
     * @param symbol
     * @return Order book
     */
    static OrderBookPtr getOrCreate(const Symbol& symbol)
    {
     return registry().getOrCreateOrderBook(symbol);
    }
    static bool contains(const Symbol& symbol) { return registry().exists(symbol); }
    static void removeFromRegistry(const Symbol& symbol) { return registry().erase(symbol); }
    static void cleanupRegistry() { return registry().cleanupRegistry(); }
    static size_t registrySize() { return registry().size(); }

   /**
     * @brief Process an incoming order: attempt matching, execute trades, and
     * persist any remaining resting quantity if applicable.
     *
     * Matching logic:
     * 1. Try to match the order against the best-priced orders on the opposite side.
     * 2. If the order is partially filled, handle remaining quantity based on order type.
     * 3. If unfulfilled (e.g., limit order not fully filled), persist it as a resting order.
     * @remarks The caller must be the worker owning this OrderBook.
     * @param order Incoming order which is looking to be matched.
     */
    void processOrder(OrderPtr order);
};



#endif //ORDERBOOK_H
