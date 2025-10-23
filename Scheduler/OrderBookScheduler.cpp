//
// Created by Vaasu Bisht on 21/10/25.
//

#include "OrderBookScheduler.h"

// void OrderBookScheduler::processOrder(OrderPtr order)
// {
//     const Worker::Id wid = getWorker(order->symbol());
//
//     submitTo(wid,
//         [this, ord = std::move(order)](const CancelToken& cTok) mutable
//         {
//             const Symbol symbol = ord->symbol();
//             const auto& ob = OrderBook::getOrCreate(symbol);
//             ob->processOrder(std::move(ord));
//         },
//         "desc");
// }

void OrderBookScheduler::processOrder(OrderPtr order)
{
    const Worker::Id wid = getWorker(order->symbol());

    // move-only lambda that owns order
    auto move_only_lambda = [this, ord = std::move(order)](const CancelToken& cTok) mutable
    {
        const Symbol symbol = ord->symbol();
        const auto& ob = OrderBook::getOrCreate(symbol);
        ob->processOrder(std::move(ord)); // pass ownership if processOrder expects OrderPtr
    };

    // wrap in a shared_ptr to make it copyable for std::function storage
    auto lam_ptr = std::make_shared<decltype(move_only_lambda)>(std::move(move_only_lambda));

    // submit a copyable lambda that invokes the real lambda via shared_ptr
    submitTo(wid,
        [lam_ptr](const CancelToken& cTok)
        {
            (*lam_ptr)(cTok);
        },
        "desc");
}

