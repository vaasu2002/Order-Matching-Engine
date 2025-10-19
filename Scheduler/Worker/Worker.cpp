//
// Created by Vaasu Bisht on 19/10/25.
//

#include "Worker.h"

Worker::Worker(const std::string& id):mId(std::move(id)), mStop(false){}

void Worker::start()
{
    std::lock_guard<std::mutex> lk(mThreadMutex);
    if(mThread.joinable())
    {
        return;
    }
    mThread = std::thread([this]
    {
        run();
    });
}


void Worker::shutdown()
{
    if(mStop)
    {
        return;
    }
    mStop = true;
}

void Worker::run()
{
    while(true)
    {
        Task t;
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);

            // Lock is released until there is task in queue to be processed and thread goes to waiting state
            // until either queue has a task or stop signal is received.
            mCv.wait(lock, [this]
            {
                return mStop || !mQueue.empty(); // Any one being true means gaining lock again and going forward.
            });

            // If stop signal is received and there is something to read exit the loop
            if(mQueue.empty() && mStop)
            {
                return;
            }

            // Checking if queue is empty
            if(mQueue.empty()) continue;

            // Adding task in queue
            t = std::move(mQueue.front());
            mQueue.pop();
            mRunningTasks.insert(t.id);
        }

    }
}

