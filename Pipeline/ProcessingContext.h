// Created by Vaasu Bisht on 13/11/25.

#pragma once

#include "OrderBook/OrderTracker/OrderTracker.h"
#include <memory>
#include <string>

/**
 * @struct ProcessingContext
 * 
 * @brief Context carried along the order processing pipeline.
 * 
 * This object groups the mutable state and read/write helpers that processors
 * in the pipeline may need while validating and executing an order.
 * 
 * Notes:
 *  - `order` is a mutable reference to the incoming order being processed.
 *  - `oppTracker` is a reference to the order-tracker representing the
 *    opposite side (tracker) used for matching.
 *  - `cond` describes the matching condition (price limit, required depth,
 *    quantity constraints, etc.). It's provided as part of the context so
 *    downstream stages have a single source of truth for match rules.
 *  - `matchedQty` is the cumulative quantity matched so far during this
 *    processing session.
 *  - Use `abortReason` (std::optional) to indicate failure; prefer this
 *    over a separate bool flag since presence of a reason is the single
 *    source of truth for abortion state.
 */
struct ProcessingContext {
    Order& order;                     ///< Incoming order (mutable — will be updated).
    OrderTracker& oppTracker;         ///< Opposite-side tracker / book used for matches.
    Condition cond;                   ///< Matching condition (qty, price limit, depth, etc.).

    // - std::nullopt     => not aborted
    // - non-empty string => aborted with reason
    std::optional<std::string> abortReason;

    // Constructor: take Condition by value and move into member for efficiency.
    ProcessingContext(Order& o, OrderTracker& t, Condition c = {})
        : order(o), oppTracker(t), cond(std::move(c)) {}

    // <============= Helpers =============>

    bool aborted() const noexcept 
    { 
        return abortReason.has_value(); 
    }

    void addAbortionReason(std::string reason){
        if(aborted()){
            *abortReason += ",";
            *abortReason += std::move(reason);
        }
        else{
            abortReason = std::move(reason);
        }
    }
};