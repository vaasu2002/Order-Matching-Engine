#include "Scheduler/OrderBookScheduler.h"
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
    auto o1 = Order::MakeLimit(1, Side::BUY, Quantity{50}, Symbol{"TESLA"}, Price{17500});
    auto o2 = Order::MakeLimit(2, Side::BUY, Quantity{50}, Symbol{"TESLA"}, Price{17500});
    auto mp = std::unordered_map<Symbol,std::string>();
    mp["TESLA"]  = "worker_0";
    mp["APPLE"]  = "worker_1";

    OrderBookScheduler bookScheduler("worker",3,mp);

    bookScheduler.start();
    bookScheduler.processOrder(std::move(o1));
    // for(int i=0;i<10000000000;i++){}
    bookScheduler.processOrder(std::move(o2));
    int x;
    std::cin>>x;

    // auto* priceLevel = new PriceLevel(100);
    // priceLevel->addOrder(std::move(o1));
    // PriceLevel::MatchResult x = priceLevel->matchOrders(99);


    return 0;
}