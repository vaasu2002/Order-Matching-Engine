// Created by Vaasu Bisht on 13/11/25.


#pragma once

#include "ProcessingContext.h"


class Handler{
protected:
    std::shared_ptr<Handler> mNext;

    /**
     * @brief Executed the main logic.
     */
    virtual void process(ProcessingContext& ctx) = 0;

public:
    virtual ~Handler() = default;

    void setNext(std::shared_ptr<Handler> next){
        mNext = next;
    }

    /**
     * @brief Execute the current handler and pass on to next
     * handler if succefull.
     */
    bool run(ProcessingContext& ctx) {
        process(ctx);
        if (mNext)
        {
            return mNext->run(ctx);
        }
        return true;
    }
};