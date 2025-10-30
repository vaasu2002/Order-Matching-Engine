#pragma once

#ifndef THREAD_ASSIGNMENT_MANAGER_H
#define THREAD_ASSIGNMENT_MANAGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>
#include <vector>
#include <optional>
#include <memory>
#include "./OrderBook/Order/Order.h"
#include "./Scheduler/OrderBookScheduler.h"


struct MetricSample{
    Symbol symbol;
    double tradesPerSecond;
    double avgOrderSize;
    std::chrono::steady_clock::time_point ts = std::chrono::steady_clock::now();
};

class ThreadAssignmentManager{
    // Default configrations
    struct Config{
        std::chrono::milliseconds rebalanceInterval{30*1000}; // 30s
        double shortAlpha{0.3};
        double longAlpha{0.02};
        double shortLongMix{0.6}; // mix of short vs long in score
        double minMoveImprovementPct{10.0}; // require >= 10% improvement
        std::chrono::seconds symbolCooldown{60}; // 60s cooldown between moves
        double minLoadToConsider{0.01}; // ignore tiny symbols (relative units)
    };    

    struct SymbolState{
        MetricSample lastSample;
        bool pending{false};
        double shortEwma{0.0};
        double longEwma{0.0};
        double loadScore{0.0};
        std::chrono::steady_clock::time_point lastUpdate = 
            std::chrono::steady_clock::time_point::min();
    };

    std::shared_ptr<OrderBookScheduler> mScheduler;
    Config mCfg;
    std::unordered_map<Symbol, SymbolState> mStates;
    std::mutex mMtx;
    std::condition_variable mCv;
    std::thread mThread;
    std::atomic<bool> mStopFlag;

    // main thread
    void run(){
        auto nextRebalance = std::chrono::steady_clock::now() + mCfg.rebalanceInterval;

        while(!mStopFlag.load()){
            std::unique_lock<std::mutex> lk(mMtx);
            mCv.wait_until(lk, nextRebalance, [&](){
                return mStopFlag.load() || hasPendingSample();
            });

            if (mStopFlag.load()) break;

            // Consume pending samples, update EWMA per-symbol
            for(auto &kv : mStates){
                auto &sym = kv.first;
                auto &st = kv.second;
                if(!st.pending){
                    continue;
                }
                updateEwma(st, st.lastSample);
                st.pending = false;
            }

            auto now = std::chrono::steady_clock::now();
            if(now >= nextRebalance){
                // snapshot states for rebalance (release lock while performing heavy work)
                auto snapshot = snapshotLoad();
                lk.unlock();
                performRebalance(snapshot);
                lk.lock();
                nextRebalance = std::chrono::steady_clock::now() + mCfg.rebalanceInterval;
            }
        }
    }

    bool hasPendingSample(){
        for (auto &kv : mStates){
            if (kv.second.pending){
                return true;
            }
        }
        return false;
    }

    void updateEwma(SymbolState &st, const MetricSample &sample) {
        // convert the sample to a single scalar s (tunable)
        double s = 2.0 * sample.tradesPerSecond + (sample.avgOrderSize / 1024.0);
        
        // initialize if first time
        if (st.shortEwma == 0.0 && st.longEwma == 0.0) 
        {
            st.shortEwma = s;
            st.longEwma = s;
        } 
        else 
        {
            st.shortEwma = mCfg.shortAlpha * s + (1.0 - mCfg.shortAlpha) * st.shortEwma;
            st.longEwma  = mCfg.longAlpha  * s + (1.0 - mCfg.longAlpha)  * st.longEwma;
        }
        // Calculating load score
        st.loadScore = mCfg.shortLongMix * st.shortEwma + (1.0 - mCfg.shortLongMix) * st.longEwma;
    }

    // returns vector of (symbol, loadScore)
    std::vector<std::pair<Symbol,double>> snapshotLoad(){
        std::vector<std::pair<Symbol,double>> out;
        out.reserve(mStates.size());
        for (auto &kv : mStates) {
            const auto &sym = kv.first;
            const auto &st = kv.second;
            out.emplace_back(sym, st.loadScore);
        }
        return out;
    }

    // actual transfer logic
    void performRebalance(const std::vector<std::pair<Symbol,double>> &snapshot){
        std::cout<<"Performing rebalance logic......"<<std::endl;
    }
public:

    ThreadAssignmentManager(std::shared_ptr<OrderBookScheduler> orderBookScheduler):mScheduler(std::move(orderBookScheduler)){}

    void start(){
        mStopFlag.store(false);
        mThread = std::thread(&ThreadAssignmentManager::run, this); // todo: Start with production
    }

    void shutdown(){
        mStopFlag.store(true);
        mCv.notify_all();
        if(mThread.joinable()){
            mThread.join();
        }
    }

    // called by metrics ingest
    // thread-safe
    void submitSample(const MetricSample& sample){
        std::lock_guard<std::mutex> g(mMtx);
        auto &st = mStates[sample.symbol];
        st.lastSample = sample;
        st.lastUpdate = sample.ts;
        st.pending = true;
        mCv.notify_all();
    }
};

#endif //THREAD_ASSIGNMENT_MANAGER_H