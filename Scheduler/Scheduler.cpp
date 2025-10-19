//
// Created by Vaasu Bisht on 19/10/25.
//

#include "Scheduler.h"

#include "Worker/Worker.h"

void Scheduler::createWorker(const std::string& id)
{
    std::lock_guard<std::mutex> lk(mLock);

    // Check if worker with id already exists.
    if(mWorkers.contains(id))
    {
        throw std::runtime_error("Worker: "+id+" already exists");
    }
    mWorkers.emplace(id, std::make_unique<Worker>(id));
}

void Scheduler::createWorkers(const std::string& prefix, const size_t cnt)
{
    {
        std::lock_guard<std::mutex> lock(mLock);
        if(mWorkers.size())
        {
            mWorkers.clear();
        }
    }
    for(size_t i = 0 ; i < cnt ; i++)
    {
        createWorker(prefix+"_"+std::to_string(i));
    }
}

void Scheduler::start()
{
    std::lock_guard<std::mutex> lock(mLock);
    for (auto& [_, worker] : mWorkers)
    {
        worker->start();
    }
}

