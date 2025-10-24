//
// Created by Vaasu Bisht on 20/10/25.
//

#include "OrderTracker.h"

#include <iostream>
#include <valarray>

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
    std::cout<<"isEmpty() price level: "<<priceLevel->isEmpty()<<std::endl;
    std::cout<<"Order Side(BUY): "<<(mSide == Side::BUY)<<std::endl;
    mOrderLocationMap[id] = std::make_pair(price, orderIt);
}

void OrderTracker::matchOrder(Quantity& qty, Price min, Price max)
{
    auto it = mPriceLevelMap.begin();

    while(it != mPriceLevelMap.end() && qty>0 )
    {
        const auto& pl = it->second;
        const Price price = it->first;
        if (!pl || pl->isEmpty() || price>=max || price<=min){
            // Price level is empty, cannot do matching.
            return;
        }

        // todo: handle trades.
        auto [remaining, trades] = pl->matchOrders(qty);
        qty = remaining;
        it++;
    }
}