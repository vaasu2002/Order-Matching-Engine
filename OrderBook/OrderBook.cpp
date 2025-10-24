//
// Created by Vaasu Bisht on 21/10/25.
//

#include "OrderBook.h"

#include <iostream>
#include <valarray>

OrderBook::OrderBook(Symbol symbol):
mSymbol(std::move(symbol))
{
    mTrackerStore.insert({Side::BUY,Tracker(Side::BUY)});
    mTrackerStore.insert({Side::SELL,Tracker(Side::SELL)});
}


// todo: separate MARKET and LIMIT orders.
// current logic only has LIMIT order.
void OrderBook::matchOrder(const OrderRawPtr order)
{
    Price maxPrice,minPrice;
    Quantity& openQty = order->openQty();
    const Side oppSide = (order->side() == Side::BUY) ? Side::SELL : Side::BUY;

    // OrderTracker of opposite side
    const auto& it = mTrackerStore.find(oppSide);

    // Attempt the match order with resting orders of opposite side.
    if(order->side() == Side::SELL)
    {
        maxPrice = 10000000000; // SOME BIG ASS NUMBER
        minPrice = order->price();
    }
    else // BUY SIDE
    {
        maxPrice = order->price();
        minPrice = 0;
    }
    it->second.matchOrder(openQty,minPrice,maxPrice);
    order->updateOpenQty(openQty);
}
void OrderBook::addRestingOrder(OrderPtr order)
{
    // auto tracker = mTrackerStore[order->side()];
    const auto& it = mTrackerStore.find(order->side());
    it->second.addOrder(std::move(order));
}

void OrderBook::processOrder(OrderPtr order)
{
    // Order is tried to match and then order is
    mStats.totalOrdersAdded++;
    mStats.totalVolume+=order->openQty();

    matchOrder(order.get());

    if(order->status() == Status::FULFILLED)
    {
        return;
    }
    if(order->type() == LIMIT)
    {
        std::cout<<"adding resting order"<<std::endl;
        addRestingOrder(std::move(order));
    }
}

