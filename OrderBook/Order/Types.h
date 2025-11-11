//
// Created by Vaasu Bisht on 20/10/25.
// Updated by Vaasu Bisht on 11/11/25.

#pragma once
#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <memory>
#include <cstdint>
#include <chrono>
#include <utility>

using Price = int64_t;
using Quantity = uint64_t;
using OrderId = uint64_t;
using Count = uint64_t;
using Symbol = std::string;
using Timestamp = std::chrono::system_clock::time_point;

enum Side
{
    BUY,
    SELL
};

enum Type : char{
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};

enum Status
{
    PENDING,
    CANCELLED,
    FULFILLED,
    PARTIALLY_FILLED
};

enum TIF : uint32_t {
    DAY = 0,
    ALL_OR_NONE = 1 << 0, // 1
    IMMEDIATE_OR_CANCEL = 1 << 1, // 2
    FILL_OR_KILL = (ALL_OR_NONE | IMMEDIATE_OR_CANCEL), // 3
    GOOD_TILL_CANCELED = 1 << 2, // 4
    DEFAULT = DAY
};

#endif //TYPES_H
