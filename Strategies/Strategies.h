// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "OrderBook/OrderTracker/OrderTracker.h"
#include <memory>

// <===================== Type strategies =====================>

/**
 * @interface ITypeStrategy
 * @brief 
 * Base strategy interface for preparing trading conditions depending on order type.
 */
class ITypeStrategy {
protected:
    /**
     * @brief Creates a default Condition populated with generic
     * values for any order.
     * - qty:     set to the order's remaining open quantity
     * - depthLimit: set to MAX (indicating no depth restriction)
     */
    Condition getDefaultCondition(const Order& order)
    {
        Condition c;
        c.qty = order.openQty();
        c.depthLimit = MAX;
        return c;
    }
public:

    virtual ~ITypeStrategy() = default;
    virtual Condition prepareCondition(const Order& order) = 0;

    /**
     * @brief Finalizes the order after a match attempt.
     *
     * @param order       Order being finalized.
     * @param matchedQty  Quantity actually matched during this attempt.
     *
     * @pre Use this after TIF finalization
     * Based in finalization of order status here it will be done
     */
    virtual void finalize(Order& order) { (void)order; }
};

/**
 * @brief Strategy for limit orders. Assigns a price limit equal to the order's
 * specified price.
 */
class LimitStrategy : public ITypeStrategy {
public:

    /**
     * @brief Must be implemented by each strategy to prepare a
     * specific Condition based on the order's type.
     */
    Condition prepareCondition(const Order& order) override 
    {
        Condition c = getDefaultCondition(order);
        c.priceLimit = order.price();
        return c;
    }
};

/**
 * @brief Strategy for market orders. Market orders do not specify a price.
 * Instead, they are allowed to match at any reasonable market price. To represent
 * this "no price limit" behavior:
 * - Buy orders use PRICE_MAX as a symbolic upper bound.
 * - Sell orders use 0 as a symbolic lower bound.
 * 
 * These values are not actual expected trading prices; they simply indicate that the
 * engine should not restrict execution by price.
 */
class MarketStrategy : public ITypeStrategy {
public:
    Condition prepareCondition(const Order& order) override 
    {
        Condition c = getDefaultCondition(order);
        c.priceLimit = (order.side() == Side::BUY) ? PRICE_MAX : 0;
        return c;
    }

    void finalize(Order& order) override 
    {
        if(order.status() == Status::PENDING){
            order.updateStatus(Status::CANCELLED);
        }
        else if(order.status() == Status::PARTIALLY_FILLED){
            order.updateStatus(Status::PARTIAL_FILL_CANCELLED);
        }
    }
};


// <===================== TIF strategies =====================>

/**
 * @interface ITifStrategy
 * @brief Interface for Time-In-Force (TIF) strategies.
 *
 * TIF strategies determine how an order behaves in terms of lifespan
 * and matching requirements (GTC, IOC, FOK, AON, etc.).
 */
class ITifStrategy {
public:
    virtual ~ITifStrategy() = default;

    /**
     * @brief Optional hook allowing the TIF to adjust the matching condition.
     *
     * Default behavior does nothing. Strategies may override this to restrict
     * or alter matching behavior (e.g., IOC depth = 1).
     */
    virtual void adjustCondition(Condition& cond, const Order& order) 
    { 
        (void)cond; 
        (void)order; 
    }

    /**
     * @brief Finalizes the order after a match attempt.
     *
     * @param order       Order being finalized.
     * @param matchedQty  Quantity actually matched during this attempt.
     *
     * @note Must update open quantity and status according to TIF rules.
     */
    virtual void finalize(Order& order, Quantity remainingQty) = 0;
};

/**
 * @brief Strategy for Good Till Canceled (GTC) Order.
 */
class GtcStrategy : public ITifStrategy {
public:
    void finalize(Order& order, Quantity remainingQty) override 
    {
        order.updateOpenQty(remainingQty);
        if(remainingQty == 0)
        {
            order.updateStatus(Status::FULFILLED);
        }
        else
        {
            order.updateStatus(Status::PARTIALLY_FILLED);
        }
    }
};

// Strategy Day is similar as Good Till Canceled (GTC)
using DayStrategy = GtcStrategy;

/**
 * @brief Strategy for Immediate or Cancel (IOC) Order
 * Fill what you can immediately; any unfilled remainder is cancelled.
 * @todo Extract IOC Depth Limit from config.xml
 */
class IocStrategy : public ITifStrategy {
public:
    // Ioc orders will have different maximum depth according to exchange policy.
    void adjustCondition(Condition& cond, const Order& /*order*/) override 
    {
        cond.depthLimit = 1;
    }
    
    // Immediate or Cancel (IOC) order allows for a partial match, 
    // and any unfilled portion of the order is immediately and automatically 
    // cancelled, not stored in the order book. 
    void finalize(Order& order, Quantity remainingQty) override {
        if(remainingQty == 0) 
        {
            // Order fully matched.
            order.updateOpenQty(0);
            order.updateStatus(Status::FULFILLED);
        } 
        else 
        {
            // Order partically matched, 
            order.updateStatus(Status::CANCELLED);
        }
    }
};

/**
 * @brief Strategy for All Or None (AON) Order.
 * Only allow execution when entire quantity can be matched, otherwise remain pending.
 * @note Partial matches must not be applied.
 */
class AonStrategy : public ITifStrategy {
public:
    void finalize(Order& order, Quantity remainingQty) override {
        if (remainingQty == 0)
        {
            order.updateOpenQty(0);
            order.updateStatus(Status::FULFILLED);
        }
        else 
        {
            order.updateStatus(Status::PENDING); 
        }
    }
};


/**
 * @brief Strategy for Fill or Kill (FOK) Order.
 * Must be filled completely immediately or canceled (no partial fills allowed).
 */
class FokStrategy : public ITifStrategy {
public:
    void finalize(Order& order, Quantity remainingQty) override {
        if(remainingQty == 0)
        {
            order.updateOpenQty(0);
            order.updateStatus(Status::FULFILLED);
        }
        else
        {
            order.updateStatus(Status::CANCELLED);
        }
    }
};

