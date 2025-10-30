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

    mAssignmentManager = std::make_shared<ThreadAssignmentManager>(mOrderBookScheduler);
    mAssignmentManager->start();

    std::cout << "mAssignmentManager started." << std::endl;


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

    if (mAssignmentManager) {
        mAssignmentManager->shutdown();
        std::cout << "mAssignmentManager shut down." << std::endl;
        mAssignmentManager.reset();
    }

    std::cout << "Application shut down successfully." << std::endl;
}

void Application::simulate()
{


    // Simulating market burst
    for (int i = 0; i < 10; ++i) {
        MetricSample s;
        s.symbol = "TESLA";
        s.tradesPerSecond = (i < 3) ? 2 : (i < 7 ? 20 : 3);
        s.tradesPerSecond = 120;
        s.ts = std::chrono::steady_clock::now();
        mAssignmentManager->submitSample(s);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // reading market burst from csv file
    std::unique_ptr<LiveMetricsProducer> metricsProducer 
        = std::make_unique<LiveMetricsProducer>(mAssignmentManager);
    metricsProducer->startFromFile("samples.csv");

    // simulating orders

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

}
