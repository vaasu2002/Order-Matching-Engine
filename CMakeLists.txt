cmake_minimum_required(VERSION 3.28.3)
project(OrderMatchingEngine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # use -std=c++20, not -std=gnu++20


add_executable(OrderMatchingEngine
        Scheduler/Worker/Task.h
        Scheduler/Worker/Worker.cpp
        Scheduler/Worker/Worker.h
        Scheduler/Scheduler.h
        Scheduler/Scheduler.cpp
        main.cpp
        OrderBook/Order/Types.h
        OrderBook/Order/Order.h
        OrderBook/Order/Validation.h
        OrderBook/Order/Validation.h
        OrderBook/Order/Validation.cpp
        OrderBook/PriceLevel/PriceLevel.cpp
        OrderBook/PriceLevel/PriceLevel.h
        OrderBook/OrderTracker/OrderTracker.cpp
        OrderBook/OrderTracker/OrderTracker.h
        OrderBook/OrderBook.cpp
        OrderBook/OrderBook.h
        OrderBook/OrderBook_Registry.cpp
)