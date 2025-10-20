#include "Scheduler/Scheduler.h"
#include "OrderBook/Order/Order.h"

#include <iostream>
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
    auto o1 = Order::MakeLimit(1, Side::BUY, Quantity{100}, Symbol{"SAMPLE"}, Price{17500});
    auto o2 = Order::MakeMarket(2, Side::SELL, Quantity{50}, Symbol{"SAMPLE"});
    std::cout<<"Working..."<<std::endl;
    return 0;
}