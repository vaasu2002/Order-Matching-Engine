//
// Created by Vaasu Bisht on 20/10/25.
//

#pragma once

#ifndef PRICE_LEVEL_H
#define PRICE_LEVEL_H

#include "../Order/Order.h"


/**
 * @brief Represents a single price point.
 * Each PriceLevel object maintains a list of orders at that price. 
 * 
 * @details
 *  - Orders are stored in FIFO by entry time.
 *  - Think of an order book like a building with floors, where each floor represents a different price.
 *  - Handles the logic matching, when price level is verified by OrderTracker
 */
class PriceLevel{
public:
    using OrderList = std::vector<OrderPtr>;
    using OrderIterator = typename OrderList::iterator;
private:
    Price mPrice; /// > Price to which this PriceLevel object corresponds.
    OrderList mOrders; /// > List of resting order at this price level.
    Quantity mTotalQuantity; /// > Total quantity of all units at this price. (liquidity at this price range)
    Count mOrderCount; /// > Total number of pending orders at this price.
public:
    /* Constructor */
    explicit PriceLevel(Price price);

    // Getters
    [[nodiscard]] Price getPrice() const {
        return mPrice;
    }
    [[nodiscard]] Quantity getTotalQuantity() const {
        return mTotalQuantity;
    }
    [[nodiscard]] Count getOrderCount() const {
        return mOrderCount;
    }

    // Returns the list of orders at this price
    [[nodiscard]] const OrderList& getOrders() const {
        return mOrders;
    }

    [[nodiscard]] bool isEmpty() const {
        return mOrders.empty();
    }


    /**
     * @brief Adds a new order to the list of tracked orders.
     * This happens in case of unfulfilled LIMIT order
     */
    OrderIterator addOrder(OrderPtr inBoundOrder);

    /**
     * @brief Removes an order from the list of tracked orders.
     *
     * @details
     * - This is typically called when an order is fully filled or cancelled.
     * - It updates the total quantity and order count accordingly.
     */
    void removeOrder(const OrderIterator& itr);

    void updateQuantity(const OrderPtr& order, Quantity oldQty, Quantity newQty);

    /**
     * @brief Get the first order in the list
     *
     * @remarks
     * Justification of using raw pointer
     * Returns a raw pointer here because the PriceLevel (this class) retains exclusive
     * ownership of all Order objects via std::unique_ptr<Order> stored in mOrders.
     * Returning a raw pointer provides safe, non-owning access to the first order
     * (FIFO) without transferring or sharing ownership. The caller can inspect or
     * modify the Order through this pointer as long as the PriceLevel remains alive
     * and does not remove the Order from mOrders. This design avoids unnecessary
     * shared_ptr reference counting overhead while still allowing safe access to
     * the managed object.
     * @return Raw pointer to the first order
     */
    [[nodiscard]] OrderRawPtr frontOrder() const;

    /**
     * @brief
     * Details of a single trade execution i.e. which resting order was hit,
     * the quantity traded, and the execution price.
     * @todo Add timestamp
     */
    struct MatchedTrade { OrderId restingOrderId{}; Quantity qty{}; Price price{};};

    /**
     * Aggregate of all trades generated during the matching process for one incoming order.
     */
    struct MatchResult { std::vector<MatchedTrade> trades; };

    /**
     * @brief Attempts to match up to `qty` units of an incoming (opposite-side) order
     *        against the resting orders in this price level.
     * 
     * This function is invoked when an inbound order is eligible to trade with the
     * resting orders at the current price level.
     * 
     * @details
     * Iterates over all resting orders at this price level, consuming as much of the
     * requested quantity (`qty`) as possible until either:
     * - the inbound order is fully filled, or
     * - all resting orders at this level are exhausted.
     * 
     * It then generates a list of all individual trade executions that occurred
     * during matching
     * 
     * @param[in,out] reqQty Quantity of the inbound order to match. Updated to remaining quantity.
     * 
     * @attention
     * The parameter `reqQty` is passed by reference and will be decremented in place
     * to reflect the remaining unfilled quantity of the inbound order after matching.
     * 
     * @return List of trades executed at this price level
     */
    MatchResult matchOrders(Quantity& reqQty);
};



#endif //PRICE_LEVEL_H
