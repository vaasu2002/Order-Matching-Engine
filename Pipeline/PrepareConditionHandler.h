// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "../Strategies/Strategies.h"
#include "../Strategies/StrategyCache.h"

#include "Handler.h"

// Prepares the matching condition based on order type.
class PrepareConditionHandler: public Handler
{
protected:
    void process(ProcessingContext& ctx) override {
        if(ctx.aborted()){
            // Skip this step if order is aborted.
            return;
        }
        auto typeStrat = StrategyCache::getTypeStrategy(ctx.order.type());
        ctx.cond = typeStrat->prepareCondition(ctx.order);
    }
};