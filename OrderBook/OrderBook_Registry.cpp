//
// Created by Vaasu Bisht on 21/10/25.
//

#include "OrderBook.h"

OrderBook::OrderBookPtr OrderBook::Registry::createOrderBook(const Symbol& symbol)
{
    // Caller must hold unique_lock on mtx. Creates shared_ptr and stores weak_ptr.
    std::cout<<"FFinally creating the order book"<<std::endl;
    auto sp = std::make_shared<OrderBook>(symbol);
    registry[symbol] = sp; // store weak_ptr implicitly
    return sp;
}

OrderBook::OrderBookPtr OrderBook::Registry::getOrderBook(const Symbol& symbol)
{
    const auto it = registry.find(symbol);
    if (it == registry.end())
    {
        return nullptr;
    }
    return it->second;
}

OrderBook::OrderBookPtr OrderBook::Registry::getOrderBookSafe(const Symbol& symbol)
{
    std::shared_lock<std::shared_mutex> rlk(mtx);
    return getOrderBook(symbol);
}

OrderBook::OrderBookPtr OrderBook::Registry::getOrCreateOrderBook(const Symbol& symbol)
{
    // Fast (read) path - shared lock
    if (const auto ob  = getOrderBookSafe(symbol); ob) {
        return ob;
    }
    std::cout<<"Order book not found in slow path"<<std::endl;

    // Slow path: unique (write) lock + recheck
    // Some other thread in meanwhile might have created the order book.
    std::unique_lock<std::shared_mutex> wlk(mtx);
    if(auto ob = getOrderBook(symbol); ob)
    {
        return ob;
    }

    std::cout<<"Order book not found in created by other thread"<<std::endl;

    // create (under write lock)
    return createOrderBook(symbol);
}

size_t OrderBook::Registry::size() const
{
    return registry.size();
}

bool OrderBook::Registry::exists(const Symbol& symbol) const
{
    return registry.contains(symbol);
}

void OrderBook::Registry::erase(const Symbol& symbol)
{
    std::unique_lock<std::shared_mutex> wlk(mtx);
    registry.erase(symbol);
}

void OrderBook::Registry::cleanupRegistry()
{
    std::unique_lock<std::shared_mutex> wlk(mtx);
    for(auto it = registry.begin(); it != registry.end();)
    {
        if(auto ob = it->second)
        {
            it = registry.erase(it);
        }
        else
        {
            it++;
        }
    }
}






