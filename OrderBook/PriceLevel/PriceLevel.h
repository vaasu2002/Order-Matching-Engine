//
// Created by Vaasu Bisht on 20/10/25.
//

#pragma once

#ifndef PRICE_LEVEL_H
#define PRICE_LEVEL_H

#include "../Order/Order.h"


/**
 * @brief Represents a single price point.
 *
 * @details
 * Contains all the unmatched and active orders submitted at that price.
 * Orders are stored in FIFO by entry time.
 * Think of an order book like a building with floors, where each floor represents a different price.
 */
class PriceLevel{
public:
    using OrderList = std::vector<OrderPtr>;
    using OrderIterator = typename OrderList::iterator;
private:
    Price mPrice; /// > Price to which this PriceLevel object corresponds.
    OrderList mOrders; /// > List of resting order at this price level.
    Quantity mTotalQuantity; /// > Total quantity of all orders at this price.
    Count mOrderCount; /// > Total number of orders at this price.
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
     * Returns a raw pointer here because the OrderBook (this class) retains exclusive
     * ownership of all Order objects via std::unique_ptr<Orderz> stored in mOrders.
     * Returning a raw pointer provides safe, non-owning access to the first order
     * (FIFO) without transferring or sharing ownership. The caller can inspect or
     * modify the Order through this pointer as long as the OrderBook remains alive
     * and does not remove the Order from mOrders. This design avoids unnecessary
     * shared_ptr reference counting overhead while still allowing safe access to
     * the managed object.
     * @return Raw pointer to the first order
     */
    [[nodiscard]] OrderRawPtr frontOrder() const;

    struct MatchedTrade { OrderId restingOrderId{}; Quantity qty{}; Price price{};};
    struct MatchResult { Quantity remaining; std::vector<MatchedTrade> trades; };
    /**
     * @brief Match order at this price level up a specified quantity.
     *
     * Used when an order from opposite side (inBoundOrder) at same price level comes with
     * open quantity `maxQty`.
     * Follows FIFO rule, earlier order gets filled first.
     *
     * @details
     * Iterate thought all the order in this price level until maxQty becomes zero.
     *
     * @return Amount of shares of inBoundOrder left and list of all trades
     */
    MatchResult matchOrders(Quantity maxQty);
};



#endif //PRICE_LEVEL_H
