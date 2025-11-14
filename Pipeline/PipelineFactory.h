// Created by Vaasu Bisht on 14/11/25.

#pragma once
#include "Pipeline.h"
#include "PrepareConditionHandler.h"
#include "TifAdjustHandler.h"
#include "ValidationHandler.h"
#include "ExecutionHandler.h"
#include "FinalizeHandler.h"
#include "../Strategies/StrategyCache.h"

/**
 * @brief Factory responsible for creating and wiring all handler instances
 * required for the order-processing pipeline.
 * 
 * This sets up a chain-of-responsibility where each handler performs a specific
 * step in the order lifecycle:
 * 
 * PrepareConditionHandler -> TifAdjustHandler -> ValidationHandler
 * -> ExecutionHandler -> FinalizeHandler
 * 
 * @details This use of the Chain-of-Responsibility pattern helps keep the logic
 * modular, testable, and easy to extend with new processing stages without
 * modifying existing ones.
 * 
 * @return A fully initialized Pipeline instance ready to process orders.
 */
class PipelineFactory {
public:
    static Pipeline createPipeline()
    {
        auto prepare = std::make_shared<PrepareConditionHandler>();
        auto tifAdjust = std::make_shared<TifAdjustHandler>();
        auto validation = std::make_shared<ValidationHandler>();
        auto exec = std::make_shared<ExecutionHandler>();
        auto finalize = std::make_shared<FinalizeHandler>();

        // chain: prepare -> tifAdjust -> validation -> liquidity -> exec -> finalize
        prepare->setNext(tifAdjust);
        tifAdjust->setNext(validation);
        validation->setNext(exec);
        exec->setNext(finalize);

        return Pipeline(prepare);
    }
};