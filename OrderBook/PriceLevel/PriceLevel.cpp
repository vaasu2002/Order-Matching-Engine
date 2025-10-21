//
// Created by Vaasu Bisht on 20/10/25.
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


PriceLevel::MatchResult PriceLevel::matchOrders(const Quantity maxQty)
{
    // Making fill result object
    MatchResult result; // final result
    result.remaining = maxQty;
    result.trades.reserve(mOrders.size()); // heuristic

    // Shares quantity we need to fill (pending)
    Quantity reqQty = maxQty;

    auto currRestingOrderIt = mOrders.begin();
    OrderRawPtr currRestingOrder = nullptr;

    while (currRestingOrderIt != mOrders.end() && reqQty > 0) {

        currRestingOrder = currRestingOrderIt->get();


        // Shares quantity we have for current order
        Quantity availableQty = currRestingOrder->openQty();

        // Shares quantity we can fill on both sides (resting order and inBoundOrder)
        // Case 1: availableQty(150) > reqQty(100);(order on other side is fulfilled)
        // Case 2: availableQty(10) < reqQty(60);  (resting order will be removed)
        // Case 3: availableQty(10) = reqQty(60);  (both order are fulfilled)
        const Quantity fillAmt = std::min(availableQty, reqQty);

        reqQty -= fillAmt;
        mTotalQuantity -= fillAmt;

        // Prepare trade record (timestamp now)
        MatchedTrade mt;
        mt.restingOrderId = currRestingOrder->id();
        mt.qty = fillAmt;
        mt.price = mPrice; // PriceLevel price member

        result.trades.push_back(std::move(mt));

        if (availableQty == fillAmt) {
            // Resting order fully filled
            currRestingOrder->updateOpenQty(0);
            currRestingOrder->updateStatus(Status::FULFILLED);

            // Removing the order from PriceLevel and going to next resting order
            currRestingOrderIt = mOrders.erase(currRestingOrderIt);
            mOrderCount--; //  Desc order count
        }
        else {
            // Current resting order is partially filled
            Quantity newOpen = availableQty - fillAmt;
            currRestingOrder->updateOpenQty(newOpen);
            currRestingOrder->updateStatus(Status::PARTIALLY_FILLED);
            currRestingOrderIt++;
        }
    }

    result.remaining = reqQty;

    return result;
}
