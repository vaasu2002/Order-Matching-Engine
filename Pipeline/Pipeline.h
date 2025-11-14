// Created by Vaasu Bisht on 13/11/25.


#pragma once
#include "Handler.h"


/**
 * @class Pipeline
 * @brief Executes an ordered chain of handlers (processing steps) for an order.
 * 
 * The Pipeline stores the head of a linked-list chain of Handler objects. Each Handler 
 * performs a specific stage of order processing (e.g., condition preparation, TIF
 * adjustment, validation, liquidity checking, execution, and finalization).
 * 
 * @details
 * Calling `process()` starts execution from the head handler, and each handler
 * passes control to the next via Handler::run(). The chain stops when:
 * - A handler returns false (aborting the pipeline), or
 * - No further handlers are available.
 */
class Pipeline {
    std::shared_ptr<Handler> mHead; ///< First handler in the chain
public:
    /** @brief Default constructor: creates an empty pipeline */
    Pipeline() = default;

    /**
     * @brief Construct a pipeline with a given head handler.
     * @param head Shared pointer to the first handler in the chain.
     */
    explicit Pipeline(std::shared_ptr<Handler> head) : mHead(std::move(head)) {}

    /**
     * @brief Set the head handler (starting point) of the pipeline.
     * @warning If a head handler is already set, it will be replaced.
     */
    void setHead(std::shared_ptr<Handler> head) 
    { 
        mHead = std::move(head); 
    }

    /**
     * @brief Execute the pipeline with the given processing context.
     *
     * @param ctx The ProcessingContext passed through all handlers.
     * @return true if the pipeline completed successfully (no handler aborted),
     *         false if any handler returned false (indicating abort).
     */
    bool process(ProcessingContext& ctx) 
    {
        if (!mHead)
        {
            return true; // No handlers, pipeline trivially succeeds
        }
        return mHead->run(ctx);
    }
};
