//
// Created by Vaasu Bisht on 20/10/25.
// Updated by Vaasu Bisht on 12/11/25.
//


/**
 * IMPORTANT CHANGES & REASONING
 * -----------------------------
 * - Introduced `Condition` as a unified input descriptor for matching.
 * - Removed direct TIF (Time-in-Force) and Order Type logic from `OrderTracker`.
 * - `OrderTracker` now focuses solely on executing matches based on the provided `Condition`.
 * - TIF and order-type–specific behavior (e.g., IOC, FOK, GTC, LIMIT, MARKET) 
 *   is now handled externally when constructing the `Condition` object.
 *
 * REASONING:
 * - Decouples matching logic from TIF/order-type semantics.
 * - Simplifies `OrderTracker` and improves maintainability.
 * - Enables easy extension for new TIF types or order behaviors 
 *   without modifying the core matching engine.
 * - Promotes a clear separation of responsibilities:
 *      → Higher-level components decide *what* to match (via Condition).
 *      → `OrderTracker` decides *how* to execute that match efficiently.
 */

#ifndef ORDERTRACKER_H
#define ORDERTRACKER_H

#include "../PriceLevel/PriceLevel.h"
#include <map>

/**
 * @struct PriceComparator
 * @brief Custom comparator for price-based map ordering
 *
 * In the BUY side: High price has priority. eg: [200,143,100,43,24,3] High to Low 
 * In the SELL side: Low price has priority. eg: [34,45,55,59,124,332] Low to High 
 */
struct PriceComparator
{
    bool isBuySide;
    explicit PriceComparator(const bool isBuySide = false): isBuySide(isBuySide){}
    bool operator()(const Price a, const Price b) const
    {
        return isBuySide ? a>b : a<b;
    }
};

struct Condition{
    int qty; /// > target quantity to matched
    int priceLimit; /// >
    int depthLimit; /// > how many price level allowed to walk
};

/**
 * @class OrderTracker
 * @brief 
 * Manages all price levels and orders for one side (bid or ask) of the order book.
 * 
 * This class owns all the PriceLevel objects belonging to one side of the book.
 * It provides fast lookup, insertion, and removal of orders by maintaining:
 * - A sorted map of Price → PriceLevel (for price-time priority)
 * - A cache mapping OrderId → (Price, Iterator) for fast order deletion and modification
 */
class OrderTracker {
    
    using PriceLevelPtr = std::shared_ptr<PriceLevel>;
    using PriceLevels = std::map<Price, PriceLevelPtr, PriceComparator>;

    /// Cache mapping OrderId → (Price, iterator in PriceLevel order list)
    using OrderLocatorMap = std::map<OrderId, std::pair<Price, typename PriceLevel::OrderIterator>>;

    Side mSide; ///< The side (Buy/Sell) that this tracker represents
    OrderLocatorMap mOrderLocator; ///< Fast access cache for locating orders by ID
    PriceLevels mPriceLevels; ///< All active price levels for this side, sorted by price

    /**
     * @brief Get the PriceLevel object for the given price.
     * @return Returns the PriceLevel if it exists, otherwise nullptr.
     */
    PriceLevelPtr getPriceLevel(Price price);

    /**
     * @brief Create a PriceLevel for the given price.
     * @warning This method does not check for existence. It unconditionally
     * creates a new PriceLevel. Prefer using ensurePriceLevel() instead.
     * It replaces if new instance inside PriceLevelMap, might lead to
     * losing information.
     *
     * @details
     * Made to follow SRP design principle and this method is used inside
     * ensurePriceLevel() to separate the PriceLevel creation logic.
     */
    PriceLevelPtr createPriceLevel(Price price);

    bool isPriceEligibleForMatch(int levelPrice, int limitPriced);

public:
    /** @brief Constructor */
    explicit OrderTracker(const Side side);

    /**
     * @brief Ensure a PriceLevel exists for the given price. If it does not exist, a new one will be
     * created and returned.
     *
     * @remarks
     * It uses createPriceLevel() and uses it safely only after checking that PriceLevel of given price
     * does not exist.
     * @return
     */
    PriceLevelPtr getOrCreatePriceLevel(Price price);

    /**
     * @brief Add order to its respective PriceLevel.
     * @param order
     */
    void addOrder(OrderPtr order);

    /**
     * @brief Executes trades by matching an incoming order against the best-priced
     * resting orders.
     *
     * Consumes liquidity from the best-priced level on the opposite side of the book,
     * up to the specified quantity.
     */
    void matchOrder(Condition& condition);
};



#endif //ORDERTRACKER_H
