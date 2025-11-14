// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "../Strategies/Strategies.h"
#include "../Strategies/StrategyCache.h"

#include "Handler.h"

// todo: Add liquidity(imp for AON) also in this
// todo: Remove violation of "law of demeter violation"
class ValidationHandler : public Handler {
    protected:
    void process(ProcessingContext& ctx) override {
        if (ctx.cond.qty <= 0) {
            ctx.addAbortionReason("Invalid Quantity");
        }

        if (ctx.order.type() == Type::LIMIT && ctx.order.price() <= 0) {
            ctx.addAbortionReason("Invalid limit price");
        }
    }
};