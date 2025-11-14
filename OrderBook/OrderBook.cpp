//
// Created by Vaasu Bisht on 21/10/25.
// Updated by Vaasu Bisht on 14/11/25.
//

#include "OrderBook.h"
#include <iostream>
#include <valarray>

OrderBook::OrderBook(Symbol symbol):
mSymbol(std::move(symbol))
{
    // Forming order tracker for both order sides
    mTrackerStore.insert({Side::BUY,Tracker(Side::BUY)});
    mTrackerStore.insert({Side::SELL,Tracker(Side::SELL)});

    // Creating pipeline instance
    mOrderPipeline = PipelineFactory::createPipeline();
}

// current logic only has LIMIT order.
void OrderBook::matchOrder(Order& order)
{
    // Fetch order tracker of opposite side
    Tracker& oppTracker = getOrderTracker(order.oppositeSide());

    // create context (captures originalQty)
    ProcessingContext ctx(order, oppTracker);

    mOrderPipeline.process(ctx);
}

OrderBook::Tracker& OrderBook::getOrderTracker(const Side side)
{
    const auto& it = mTrackerStore.find(side);
    return it->second;
}

void OrderBook::addRestingOrder(OrderPtr order)
{
    auto tracker = getOrderTracker(order->side());
    tracker.addOrder(std::move(order));
}

void OrderBook::processOrder(OrderPtr order)
{
    // Order is tried to match and then order is
    mStats.totalOrdersAdded++;
    mStats.totalVolume+=order->openQty();

    matchOrder(*order);

    if(order->status() == Status::PENDING || order->status() == Status::PARTIALLY_FILLED){
        addRestingOrder(std::move(order));
    }
}