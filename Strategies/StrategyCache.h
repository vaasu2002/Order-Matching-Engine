// Created by Vaasu Bisht on 13/11/25.

#pragma once
#include <memory>
#include <vector>
#include "Strategies.h"

/**
 * @struct StrategyCache
 *
 * @brief Provides shared, stateless strategy singletons for order processing.
 * Reusing these instances avoids unnecessary allocations and keeps the design
 * simple and efficient. Since each order book runs on its own dedicated thread
 * and no Order object is ever shared between threads, these strategies can be
 * safely accessed concurrently without risk of data races.
 */
struct StrategyCache {
    static std::shared_ptr<ITypeStrategy> limit() {
        static std::shared_ptr<ITypeStrategy> inst = std::make_shared<LimitStrategy>();
        return inst;
    }

    static std::shared_ptr<ITypeStrategy> market() {
        static std::shared_ptr<ITypeStrategy> inst = std::make_shared<MarketStrategy>();
        return inst;
    }

    static std::shared_ptr<ITifStrategy> gtc() {
        static std::shared_ptr<ITifStrategy> inst = std::make_shared<GtcStrategy>();
        return inst;
    }

    static std::shared_ptr<ITifStrategy> day() {
        static std::shared_ptr<ITifStrategy> inst = std::make_shared<DayStrategy>();
        return inst;
    }

    static std::shared_ptr<ITifStrategy> ioc() {
        static std::shared_ptr<ITifStrategy> inst = std::make_shared<IocStrategy>();
        return inst;
    }

    static std::shared_ptr<ITifStrategy> fok() {
        static std::shared_ptr<ITifStrategy> inst = std::make_shared<FokStrategy>();
        return inst;
    }

    static std::shared_ptr<ITifStrategy> aon() {
        static std::shared_ptr<ITifStrategy> inst = std::make_shared<AonStrategy>();
        return inst;
    }

    /**
     * Returns the strategy for the given order type.
     * Falls back to DEFAULT if an unknown type is provided
     */
    static std::shared_ptr<ITypeStrategy> getTypeStrategy(Type type)
    {
        switch (type)
        {
            case Type::LIMIT:  return limit();
            case Type::MARKET: return market();
        }
        return getTypeStrategy(Type::DEFAULT_TYPE);
    }

    /**
     * Returns the Time-In-Force strategy for the given TIF.
     * Falls back to DEFAULT if an unknown TIF is provided.
     */
    static std::shared_ptr<ITifStrategy> getTifStrategy(TIF tif)
    {
        switch (tif)
        {
            case TIF::GOOD_TILL_CANCELED: return gtc();
            case TIF::IMMEDIATE_OR_CANCEL: return ioc();
            case TIF::FILL_OR_KILL: return fok();
            case TIF::ALL_OR_NONE: return aon();
        }
        return getTifStrategy(TIF::DEFAULT_TIF);
    }
};