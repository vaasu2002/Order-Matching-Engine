#include "Scheduler/Scheduler.h"
#include "OrderBook/PriceLevel/PriceLevel.h"

int main()
{
    // Constructing validation chain
    const auto chain = std::make_shared<OrderValidator>();
    chain->add(std::make_unique<QuantityValidator>());
    chain->add(std::make_unique<LimitPriceRequiredValidator>());
    chain->add(std::make_unique<StopPriceRequiredValidator>());
    // Setting default configuration
    Order::SetDefaultValidator(chain);

    // Constructing order objects
    auto o1 = Order::MakeLimit(1, Side::BUY, Quantity{50}, Symbol{"SAMPLE"}, Price{17500});

    auto* priceLevel = new PriceLevel(100);
    priceLevel->addOrder(std::move(o1));
    PriceLevel::MatchResult x = priceLevel->matchOrders(99);

    return 0;
}