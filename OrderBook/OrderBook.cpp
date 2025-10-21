//
// Created by Vaasu Bisht on 21/10/25.
//

#include "OrderBook.h"

#include <utility>

OrderBook::OrderBook(Symbol symbol):
mSymbol(std::move(symbol)),
mBidTracker(Side::BUY),
mAskTracker(Side::SELL){}



void OrderBook::matchOrder(OrderRawPtr order)
{
    // simulating order matching and updating of current order
    order->updateStatus(Status::FULFILLED);
    order->updateOpenQty(order->openQty()-1);
}
void OrderBook::addRestingOrder(OrderPtr order)
{
    if(order->side() == Side::BUY)
    {
        mBidTracker.addOrder(std::move(order));
        return;
    }
    mAskTracker.addOrder(std::move(order));
}

void OrderBook::processOrder(OrderPtr order)
{
    // Order is tried to match and then order is
    matchOrder(order.get());
    if(order->status() == Status::FULFILLED)
    {
        return;
    }
    if(order->type() == LIMIT)
    {
        addRestingOrder(std::move(order));
    }
}

