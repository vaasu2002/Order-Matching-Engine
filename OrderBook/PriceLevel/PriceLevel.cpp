//
// Created by Vaasu Bisht on 20/10/25.
// Updated by Vaasu Bisht on 11/11/25.
//

#include "PriceLevel.h"
#include <iostream>

PriceLevel::PriceLevel(Price price) : mPrice(price), mTotalQuantity(0), mOrderCount(0) {}

PriceLevel::OrderIterator PriceLevel::addOrder(OrderPtr inBoundOrder)
{
    mTotalQuantity += inBoundOrder->openQty();
    mOrderCount++;
    return mOrders.insert(mOrders.end(), std::move(inBoundOrder));
}

void PriceLevel::removeOrder(const OrderIterator& itr)
{
    if (itr != mOrders.end())
    {
        mTotalQuantity -= (*itr)->openQty();
        mOrderCount--;
        mOrders.erase(itr);
    }
}

void PriceLevel::updateQuantity(const OrderPtr& order, Quantity oldQty, Quantity newQty)
{
    order->updateOpenQty(newQty);
    mTotalQuantity += (newQty - oldQty); // O(1)
}

OrderRawPtr PriceLevel::frontOrder() const
{
    if (mOrders.empty()) {
        return nullptr;
    }
    return mOrders.front().get();
}


PriceLevel::MatchResult PriceLevel::matchOrders(Quantity& reqQty)
{
    MatchResult result; // final result

    result.trades.reserve(mOrders.size()); // heuristic: avoid frequent reallocations

    // Begin matching according to price–time priority (FIFO)
    // Start from the earliest resting order at this price level.
    auto restingIt  = mOrders.begin(); // Iterator to  resting order

    // Continue matching until either the requested quantity is fully filled
    // or there are no more resting orders available at this price level.
    while (restingIt != mOrders.end() && reqQty > 0) {

        // Pointer to the current resting order (oldest at this level).
        OrderRawPtr restingOrder = restingIt->get(); 

        Quantity unitsAvailable = restingOrder->openQty();

        // Case 1: unitsAvailable(150) > reqQty(100);(order on other side is fulfilled)
        // Case 2: unitsAvailable(10) < reqQty(60);  (resting order will be removed)
        // Case 3: unitsAvailable(10) = reqQty(60);  (both order are fulfilled)
        const Quantity fillAmt = std::min(unitsAvailable, reqQty); // Units we will take from this resting order

        // update external and level quantities
        reqQty -= fillAmt;
        mTotalQuantity -= fillAmt;

        // record trade
        MatchedTrade mt;
        mt.restingOrderId = restingOrder->id();
        mt.qty = fillAmt;
        mt.price = mPrice; // Price of this level

        result.trades.push_back(std::move(mt));

        if (unitsAvailable == fillAmt) {
            // resting order fully filled -> remove from level
            restingOrder->updateOpenQty(0);
            restingOrder->updateStatus(Status::FULFILLED);

            // Removing the order from this level
            restingIt = mOrders.erase(restingIt); // Returns next iterator
            mOrderCount--; //  Desc order count
        }
        else {
            // Partially filled
            Quantity newOpen = unitsAvailable - fillAmt;
            // Update remaining open qty and keep it in level
            restingOrder->updateOpenQty(newOpen);
            restingOrder->updateStatus(Status::PARTIALLY_FILLED);

            // incoming order must be fully filled when resting is partially filled,
            // so break out early to avoid unnecessary iterator advancement.
            break;
        }
    }

    return result;
}
