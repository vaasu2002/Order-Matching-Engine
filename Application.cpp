//
// Created by Vaasu Bisht on 24/10/25.
//

#include "Application.h"

void Application::start()
{
    // todo: remove this is just a example for testing
    auto mp = std::unordered_map<Symbol,std::string>();
    mp["TESLA"]  = "OB_Worker_0";
    mp["APPLE"]  = "OB_Worker_1";

    std::cout<<"prefix: "<<mConfig.obWorkerPrefix<<std::endl;
    std::cout<<"cnt: "<<mConfig.obWorkerCnt<<std::endl;
    mOrderBookScheduler = std::make_shared<OrderBookScheduler>(
        mConfig.obWorkerPrefix,
        mConfig.obWorkerCnt,
        mp
    );
    mOrderBookScheduler->start();

    std::cout << "OrderBookScheduler started with " << mConfig.obWorkerCnt << " workers." << std::endl;
    std::cout << "Application started successfully." << std::endl;
}


void Application::shutdown()
{
    std::cout << "Application shutting down..." << std::endl;

    if (mOrderBookScheduler) {
        mOrderBookScheduler->shutdown();
        std::cout << "OrderBookScheduler shut down." << std::endl;
        mOrderBookScheduler.reset();
    }
    std::cout << "Application shut down successfully." << std::endl;
}

void Application::simulate()
{
    auto o1 = Order::MakeLimit(1, Side::BUY, Quantity{150}, Symbol{"TESLA"}, Price{17500});
    auto o2 = Order::MakeLimit(2, Side::SELL, Quantity{50}, Symbol{"TESLA"}, Price{17400});

    mOrderBookScheduler->processOrder(std::move(o1));
    mOrderBookScheduler->processOrder(std::move(o2));

    std::cin.get();
}
