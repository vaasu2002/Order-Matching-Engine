#pragma once
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>

#include "ThreadAssignmentManager.h"

using namespace std::chrono_literals;

class LiveMetricsProducer {
public:
    struct Sample {
        uint64_t delayMs{0}; // delay from previous sample in ms
        std::string symbol;
        double msgs_per_sec{0.0};
        double trades_per_sec{0.0};
        double avg_order_size{0.0};
    };

    LiveMetricsProducer(std::shared_ptr<ThreadAssignmentManager> assignmentManager)
        : mAssignmentManager(std::move(assignmentManager)), mStop(false) {}

    ~LiveMetricsProducer() { stop(); }

    // Start producer thread reading a file (CSV format described above)
    bool startFromFile(const std::string &filepath) {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            std::cerr << "LiveMetricsProducer: failed to open " << filepath << "\n";
            return false;
        }
        // parse into memory first (so we can easily restart or inspect)
        std::vector<Sample> samples;
        std::string line;
        while (std::getline(ifs, line)) {
            trim(line);
            if (line.empty() || line[0] == '#') continue;
            auto s = parseLine(line);
            if (s) samples.push_back(*s);
        }
        ifs.close();
        if (samples.empty()) {
            std::cerr << "LiveMetricsProducer: no samples in " << filepath << "\n";
            return false;
        }
        return startFromVector(std::move(samples));
    }

    // Start with an in-memory vector of samples
    bool startFromVector(std::vector<Sample> samples) {
        if (mThread.joinable()) {
            std::cerr << "LiveMetricsProducer: already running\n";
            return false;
        }
        mStop.store(false);
        mThread = std::thread(&LiveMetricsProducer::producerLoop, this, std::move(samples));
        return true;
    }

    // Stop producer thread
    void stop() {
        mStop.store(true);
        if (mThread.joinable()) mThread.join();
    }

private:
    std::shared_ptr<ThreadAssignmentManager> mAssignmentManager;
    std::thread mThread;
    std::atomic<bool> mStop;

    static inline void trim(std::string &s) {
        // simple trim
        auto not_space = [](int ch) { return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    }

    // Parse CSV line -> Sample (expects 5 fields)
    std::optional<Sample> parseLine(const std::string &line) {
        std::istringstream ss(line);
        std::string token;
        Sample out;
        // delay_ms
        if (!std::getline(ss, token, ',')) return std::nullopt;
        trim(token);
        try { out.delayMs = static_cast<uint64_t>(std::stoull(token)); } catch(...) { return std::nullopt; }
        // symbol
        if (!std::getline(ss, token, ',')) return std::nullopt;
        trim(token); out.symbol = token;
        // msgs_per_sec
        if (!std::getline(ss, token, ',')) return std::nullopt;
        trim(token); out.msgs_per_sec = std::stod(token);
        // trades_per_sec
        if (!std::getline(ss, token, ',')) return std::nullopt;
        trim(token); out.trades_per_sec = std::stod(token);
        // avg_order_size
        if (!std::getline(ss, token, ',')) return std::nullopt;
        trim(token); out.avg_order_size = std::stod(token);
        return out;
    }

    // loop that injects samples into the 
    void producerLoop(std::vector<Sample> samples) {
        for (const auto &s : samples) {
            if (mStop.load()) break;
            if (s.delayMs > 0) {
                // break into chunks so we can stop quickly if requested
                uint64_t remaining = s.delayMs;
                const uint64_t chunk = 50; // ms
                while (remaining > 0 && !mStop.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(std::min<uint64_t>(chunk, remaining)));
                    remaining = (remaining > chunk) ? (remaining - chunk) : 0;
                }
                if (mStop.load()) break;
            }
            // build MetricSample and submit
            MetricSample ms;
            ms.symbol = s.symbol;
            ms.avgOrderSize = s.avg_order_size;
            ms.tradesPerSecond = s.trades_per_sec;
            ms.ts = std::chrono::steady_clock::now();
            // submit to assignment manager
            if (mAssignmentManager) {
                mAssignmentManager->submitSample(ms);
            }
        }
    }
};
