//
// Created by Vaasu Bisht on 20/10/25.
// Updated by Vaasu Bisht on 12/11/25.
//

#include "OrderTracker.h"

#include <iostream>
#include <valarray>

OrderTracker::OrderTracker(const Side side):mSide(side){}

OrderTracker::PriceLevelPtr OrderTracker::createPriceLevel(const Price price)
{
    auto priceLevel = std::make_shared<PriceLevel>(price);
    mPriceLevels[price] = priceLevel;
    return priceLevel;
}

OrderTracker::PriceLevelPtr OrderTracker::getPriceLevel(const Price price)
{
    const auto it = mPriceLevels.find(price);

    // Check if price level does not exists
    if(it == mPriceLevels.end())
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
    if(mOrderLocator.contains(id))
    {
        // Order already exists
        return;
    }

    const PriceLevelPtr priceLevel = getOrCreatePriceLevel(price);
    auto orderIt = priceLevel->addOrder(std::move(order));
    mOrderLocator[id] = std::make_pair(price, orderIt);
}

bool OrderTracker::isPriceEligibleForMatch(int levelPrice, int limitPriced){
    if (mSide == Side::SELL){
        // SELL: valid if buyer's offer>= seller’s limit price.
        return levelPrice <= limitPriced; 
    }
    else{
        // BUY: valid if seller’s offer ≤ buyer’s limit price.
        return levelPrice >= limitPriced; 
    }
}

void OrderTracker::matchOrder(Condition& condition)
{
    // Begin matching using price–time priority:
    // For buy orders → start from the highest price level.
    // For sell orders → start from the lowest price level.
    // The `mPriceLevels` map is already sorted appropriately by `PriceComparator`,
    // ensuring we always access the best available price first.

    // Points to the first price level (highest bid or lowest ask)
    auto it = mPriceLevels.begin();
    int currDepth = 0; // Current price level depth being processed
    Quantity unitsNeeded = condition.qty; // Remaining quantity to match for the incoming order

    while(
        unitsNeeded > 0 &&- // Still need more units to fulfill the order
        currDepth <= condition.depthLimit && // Stay within the allowed market depth
        it != mPriceLevels.end() && // No price level left to explore
        isPriceEligibleForMatch(it->first,condition.priceLimit) // Ensure price is within acceptable range
    ){

        const auto& priceLevel = it->second;

        if(!priceLevel || priceLevel->isEmpty()){
            // Price level is empty
            it++;
            currDepth++;
            continue;
        }

        // Attempt to match orders at this price level.
        // It reduces `unitsNeeded` accordingly.
        auto trades = priceLevel->matchOrders(unitsNeeded); // List of trades executed

        // Move to the next price level for further matching if needed
        it++;
        currDepth++;
    }
}