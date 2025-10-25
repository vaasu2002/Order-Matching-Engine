//
// Created by Vaasu Bisht on 24/10/25.
//

#include "Application.h"

void Application::start()
{
    // todo: remove this is just a example for testing
    auto mp = std::unordered_map<Symbol,std::string>();
    mp["TESLA"]  = "OBWorker_0";
    mp["APPLE"]  = "OBWorker_1";

    std::cout<<"prefix: "<<mConfig.obWorkerPrefix<<std::endl;
    std::cout<<"cnt: "<<mConfig.obWorkerCnt<<std::endl;

    mOrderBookScheduler = std::make_shared<OrderBookScheduler>(
        mConfig.obWorkerPrefix,
        mConfig.obWorkerCnt,
        mp
    );
    mOrderBookScheduler->start();

    std::cout << "OrderBookScheduler started with " << mConfig.obWorkerCnt << " workers." << std::endl;

    mOrderInjectorScheduler = std::make_shared<OrderInjectorScheduler>(
        mConfig.oiWorkerPrefix,
        mConfig.oiWorkerCnt,
        mOrderBookScheduler
    );
    mOrderInjectorScheduler->start();

    std::cout << "mOrderInjectorScheduler started with " << mConfig.oiWorkerCnt << " workers." << std::endl;

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
    if (mOrderInjectorScheduler) {
        mOrderInjectorScheduler->shutdown();
        std::cout << "mOrderInjectorScheduler shut down." << std::endl;
        mOrderInjectorScheduler.reset();
    }
    std::cout << "Application shut down successfully." << std::endl;
}

void Application::simulate()
{
    std::vector<std::string> messages = {
        "id=1;side=BUY;qty=100;symbol=TESLA;price=17500;type=LIMIT",
        "id=2;side=SELL;qty=50;symbol=TESLA;price=17400;type=LIMIT",
        "id=3;side=BUY;qty=30;symbol=TESLA;price=17350;type=LIMIT",
        "id=4;side=SELL;qty=70;symbol=TESLA;price=17600;type=LIMIT"
    };

    for(const auto& msg:messages)
    {
        mOrderInjectorScheduler->processIncomingOrder(msg);
    }

    std::cin.get();
}
