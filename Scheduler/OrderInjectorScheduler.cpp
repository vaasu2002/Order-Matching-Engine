

#include "OrderInjectorScheduler.h"


Worker::Id OrderInjectorScheduler::getWorkerIdForOrder() const
{
    const size_t id = mNextWorkerId++ % mWorkerCount;
    return mWorkerPrefix + "_" + std::to_string(id);
}


void OrderInjectorScheduler::processIncomingOrder(const std::string& orderMessage)
{
    const Worker::Id wid = getWorkerIdForOrder(); // Worker that will handle this order.

    // Submit the task to the injector worker
    submitTo(wid,
        [this, msg = orderMessage](const CancelToken& cTok) mutable
        {
            // Paring order message
            std::unordered_map<std::string, std::string> fields;
            std::stringstream ss(msg);
            std::string kv;
            while (std::getline(ss, kv, ';')) {
                if (auto pos = kv.find('='); pos != std::string::npos) {
                    fields[kv.substr(0, pos)] = kv.substr(pos + 1);
                }
            }

            // constructing order object

            OrderPtr order = nullptr;

            if(fields["type"] == "LIMIT")
            {
                 order =  Order::MakeLimit(
                    std::stoi(fields["id"]),
                    fields["side"] == "BUY" ? Side::BUY : Side::SELL,
                    Quantity{static_cast<unsigned long long>(std::stoull(fields["qty"]))},
                    Symbol{fields["symbol"]},
                    Price{std::stoi(fields["price"])}
                );
            }
            else
            {
                order = Order::MakeMarket(
                    std::stoi(fields["id"]),
                    fields["side"] == "BUY" ? Side::BUY : Side::SELL,
                    Quantity{static_cast<unsigned long long>(std::stoull(fields["qty"]))},
                    Symbol{fields["symbol"]}
                );
            }

            // Delegate to order book workers
            mOrderBookScheduler->processOrder(std::move(order));
        },
        "OrderInjector: parse & delegate order");
}
