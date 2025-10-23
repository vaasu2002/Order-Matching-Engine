//
// Created by Vaasu Bisht on 20/10/25.
//

#include "OrderTracker.h"

OrderTracker::OrderTracker(const Side side):mSide(side)
{
    mOrderLocationMap.clear();
    mPriceLevelMap.clear();
}

OrderTracker::PriceLevelPtr OrderTracker::createPriceLevel(const Price price)
{
    auto priceLevel = std::make_shared<PriceLevel>(price);
    mPriceLevelMap[price] = priceLevel;
    return priceLevel;
}


OrderTracker::PriceLevelPtr OrderTracker::getPriceLevel(const Price price)
{
    const auto it = mPriceLevelMap.find(price);
    if(it == mPriceLevelMap.end())
    {
        return nullptr;
    }
    auto priceLevel = it->second;
    return priceLevel;
}


OrderTracker::PriceLevelPtr OrderTracker::getOrCreatePriceLevel(Price price)
{
    if(auto priceLevel = getPriceLevel(price))
    {
        return priceLevel;
    }
    return createPriceLevel(price);
}

void OrderTracker::addOrder(OrderPtr order)
{
    if(!order) return;

    const OrderId id = order->id();
    const Price price = order->price();
    if(mOrderLocationMap.contains(id))
    {
        // Order already exists
        return;
    }

    const PriceLevelPtr priceLevel = getOrCreatePriceLevel(price);
    auto orderIt = priceLevel->addOrder(std::move(order));
    mOrderLocationMap[id] = std::make_pair(price, orderIt);
}

