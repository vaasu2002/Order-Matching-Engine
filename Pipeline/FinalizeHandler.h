// Created by Vaasu Bisht on 14/11/25.

#pragma once

#include "../Strategies/StrategyCache.h"

#include "Handler.h"

// todo: Remove violation of "law of demeter violation"
/**
 * @brief Finalizes an order after matching by updating its remaining quantity
 * and determining the appropriate final order status.
 * 
 * @pre Must be called only after the matching process has completed.
 * 
 * @attention Always finalize the TIF strategy before the type strategy.
 * TIF processing may leave an order in PENDING or `PARTIALLY_FILLED`
 * state, while market-type orders must resolve strictly to
 * `CANCELLED` or `FILLED` (never `PENDING`). Orders left in `PENDING` or 
 * `PARTIALLY_FILLED` state are placed into the order book.
 */
class FinalizeHandler : public Handler {
    protected:
    void process(ProcessingContext& ctx) override {
        auto& order = ctx.order;

        // `remainingQty` is taken from the condition object, which the order tracker
        // updated to reflect how much quantity is still left to be fulfilled.
        auto& remainingQty = ctx.cond.qty; 

        auto tifStrategy = StrategyCache::getTifStrategy(order.tif());
        auto typeStrategy = StrategyCache::getTypeStrategy(order.type());

        tifStrategy->finalize(order,remainingQty);
        typeStrategy->finalize(order);
    }
};