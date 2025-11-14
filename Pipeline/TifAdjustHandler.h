// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "../Strategies/Strategies.h"
#include "../Strategies/StrategyCache.h"

#include "Handler.h"

// todo: Remove violation of "law of demeter violation"
class TifAdjustHandler : public Handler {
    protected:
    void process(ProcessingContext& ctx) override {
        if(ctx.aborted()){
            // Skip this step if order is aborted.
            return;
        }
        auto typeStrat = StrategyCache::getTifStrategy(ctx.order.tif());
        typeStrat->adjustCondition(ctx.cond, ctx.order);
    }
};