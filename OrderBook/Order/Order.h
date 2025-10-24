//
// Created by Vaasu Bisht on 20/10/25.
//
#pragma once

#ifndef ORDER_H
#define ORDER_H

#include <stdexcept>
#include <string>
#include <memory>

#include "Types.h"
#include "Validation.h"

class Order
{
public:
    /**
     * @brief Set the process-wide default validator used by the no-arg factories.
     * Call this once during startup (before creating orders from multiple threads).
     */
    static void SetDefaultValidator(std::shared_ptr<const IValidator> v)
    {
        if (v) DefaultValidatorPtr() = std::move(v);
    }

    /**
     * @brief Get the current default validator (No-Op if none was set).
     */
    static const IValidator& DefaultValidator()
    {
        return *DefaultValidatorPtr();
    }

    // ============================ Static factories ===========================

    static std::unique_ptr<Order> MakeLimit(const OrderId id,
                           const Side side,
                           const Quantity qty,
                           Symbol symbol,
                           const Price limitPrice,
                           const IValidator& validator)
    {
        return makeAndValidate(id, side, qty, std::move(symbol),
                               Type::LIMIT, limitPrice, Price{0}, validator);
    }

    static std::unique_ptr<Order> MakeMarket(const OrderId id,
                            const Side side,
                            const Quantity qty,
                            Symbol symbol,
                            const IValidator& validator)
    {
        return makeAndValidate(id, side, qty, std::move(symbol),
                               Type::MARKET, Price{0}, Price{0}, validator);
    }

    static std::unique_ptr<Order> MakeStop(const OrderId id,
                          const Side side,
                          const Quantity qty,
                          Symbol symbol,
                          const Price stopPrice,
                          const IValidator& validator)
    {
        return makeAndValidate(id, side, qty, std::move(symbol),
                               Type::STOP, Price{0}, stopPrice, validator);
    }

    static std::unique_ptr<Order> MakeStopLimit(const OrderId id,
                               const Side side,
                               const Quantity qty,
                               Symbol symbol,
                               const Price limitPrice,
                               const Price stopPrice,
                               const IValidator& validator)
    {
        return makeAndValidate(id, side, qty, std::move(symbol),
                               Type::STOP_LIMIT, limitPrice, stopPrice, validator);
    }

    static std::unique_ptr<Order> MakeLimit(const OrderId id,
                           const Side side,
                           const Quantity qty,
                           Symbol symbol,
                           const Price limitPrice)
    {
        return MakeLimit(id, side, qty, std::move(symbol), limitPrice, DefaultValidator());
    }

    static std::unique_ptr<Order> MakeMarket(const OrderId id,
                            const Side side,
                            const Quantity qty,
                            Symbol symbol)
    {
        return MakeMarket(id, side, qty, std::move(symbol), DefaultValidator());
    }

    static std::unique_ptr<Order> MakeStop(const OrderId id,
                          const Side side,
                          const Quantity qty,
                          Symbol symbol,
                          const Price stopPrice)
    {
        return MakeStop(id, side, qty, std::move(symbol), stopPrice, DefaultValidator());
    }

    static std::unique_ptr<Order> MakeStopLimit(const OrderId id,
                               const Side side,
                               const Quantity qty,
                               Symbol symbol,
                               const Price limitPrice,
                               const Price stopPrice)
    {
        return MakeStopLimit(id, side, qty, std::move(symbol), limitPrice, stopPrice, DefaultValidator());
    }

    OrderId id()        const noexcept { return mId; }
    Side side()         const noexcept { return mSide; }
    Side oppositeSide() const noexcept { return (mSide == Side::BUY) ? Side::SELL : Side::BUY; }
    Quantity qty()      const noexcept { return mQty; }
    Quantity openQty() const noexcept { return mOpenQty; }
    Quantity& openQty() noexcept { return mOpenQty; }
    const Symbol& symbol() const noexcept { return mSymbol; }
    Status status()     const noexcept { return mStatus; }
    Type type()         const noexcept { return mType; }
    Price price()       const noexcept { return mPrice; }
    Price stopPrice()   const noexcept { return mStopPrice; }

    void updateOpenQty(const Quantity& qty)
    {
        mOpenQty = qty;
    }

    void updateStatus(const Status& status)
    {
        mStatus = status;
    }
private:

    static std::shared_ptr<const IValidator>& DefaultValidatorPtr()
    {
        // Function-local static: initialized once per process in a thread-safe manner.
        static std::shared_ptr<const IValidator> ptr = std::make_shared<NoOpValidator>();
        return ptr;
    }

    /**
     * @brief Centralized construction to validate and instantiate Order objects.
     * @remark Object creation happens here.
     * @remark Since the `Order` constructor is private, `std::make_unique` cannot be used
     *   to create the `std::unique_ptr`.
     */
    static std::unique_ptr<Order> makeAndValidate(const OrderId id,
                                 const Side side,
                                 const Quantity qty,
                                 Symbol symbol,
                                 const Type type,
                                 const Price price,
                                 const Price stopPrice,
                                 const IValidator& validator)
    {
        // Cannot use make_unique<Order> as the Order constructor is private.
        // Making constructor public violates “factory-only creation” design.
        // So raw pointer is being typecast to unique pointer here.

        auto tmp = std::unique_ptr<Order>(
            new Order{id, side, qty, std::move(symbol), type, price, stopPrice});

        if (std::string reason; !validator.validate(*tmp, reason))
        {
            if (reason.empty())
            {
                reason = "Order validation failed. Unexpected Error.";
            }
            throw std::invalid_argument(reason);
        }
        return tmp;
    }

    /** @brief Core ctor kept private to enforce factories */
    Order(const OrderId id, const Side side, const Quantity qty, Symbol symbol,
          const Type type, const Price price, const Price stopPrice)
        : mId(id),
          mSide(side),
          mQty(qty),
          mOpenQty(qty),
          mSymbol(std::move(symbol)),
          mStatus(Status::PENDING),
          mType(type),
          mPrice(price),
          mStopPrice(stopPrice)
    {}

private:
    OrderId mId;
    Side mSide;
    Quantity mQty;
    Quantity mOpenQty;
    Symbol mSymbol;
    Status mStatus;
    Type mType;
    Price mPrice;     // For LIMIT or STOP_LIMIT
    Price mStopPrice; // For STOP or STOP_LIMIT
};

using OrderRawPtr = Order*;
using OrderPtr = std::unique_ptr<Order>;

#endif // ORDER_H
