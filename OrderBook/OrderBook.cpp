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

    // Fetch order tracker of opposite side
    auto tracker = getOrderTracker(order->oppositeSide()); // opposite side's order tracker

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
    tracker.matchOrder(openQty,minPrice,maxPrice);
    updateOrder(*order,openQty);
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

    matchOrder(order.get());

    if(order->status() != Status::FULFILLED && order->type() == Type::LIMIT)
    {
        // Unfulfilled(pending / partially fulfilled) ordered are added in order book.
        addRestingOrder(std::move(order));
    }
}

void OrderBook::updateOrder(Order& order, const Quantity remainingQty)
{
    const Quantity currentQty = order.openQty();

    // If quantity is unchanged, no trade occurred.
    if(currentQty == remainingQty)
    {
        // For a Market order, no match means it should be cancelled.
        if(order.type() == Type::MARKET)
        {
            order.updateStatus(Status::CANCELLED);
        }
        return;
    }
    if (remainingQty > currentQty)
    {
        throw std::logic_error("Invalid order quantity update.");
    }

    // --- Main Logic: A Match Occurred ---
    // We now know: 0 <= remainingQty < currentQty

    // Update the quantity
    order.updateOpenQty(remainingQty);

    // Update the status
    const Status newStatus = (remainingQty == 0)
                                 ? Status::FULFILLED
                                 : Status::PARTIALLY_FILLED;
    order.updateStatus(newStatus);
}