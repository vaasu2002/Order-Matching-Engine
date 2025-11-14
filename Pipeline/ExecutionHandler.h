// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "../Strategies/Strategies.h"
#include "../Strategies/StrategyCache.h"

#include "Handler.h"

// todo: Remove violation of "law of demeter violation"
class ExecutionHandler : public Handler {
    protected:
    void process(ProcessingContext& ctx) override {
        if(ctx.aborted()){
            // No matching will be performed because the order is aborted.
            return;
        }
        ctx.oppTracker.matchOrder(ctx.cond);
    }
};