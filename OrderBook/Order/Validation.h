#pragma once
#ifndef ORDER_VALIDATOR_H
#define ORDER_VALIDATOR_H

#include <memory>
#include <string>
#include <vector>
#include "Types.h"

class Order; ///> Forward Declaration


class IValidator
{
public:
    virtual ~IValidator() = default;
    virtual bool validate(const Order& order, std::string& reason) const = 0;
};

class QuantityValidator final : public IValidator
{
public:
    bool validate(const Order& order, std::string& reason) const override;
};

class LimitPriceRequiredValidator final : public IValidator
{
public:
    bool validate(const Order& order, std::string& reason) const override;
};

class StopPriceRequiredValidator final : public IValidator
{
public:
    bool validate(const Order& order, std::string& reason) const override;
};

/**
 * @class OrderValidator
 * @brief Performs order validation before an order is instantiated.
 *
 * It implements the Chain of Responsibility (CoR) pattern, allowing multiple validation rules
 * to be linked together and executed in sequence.
 */
class OrderValidator final : public IValidator
{
public:
    /** @brief Add a validator */
    void add(std::unique_ptr<IValidator> v)
    {
        mChain.emplace_back(std::move(v));
    }

    /** @brief Checks for validation. Exits even if one validation fails. */
    bool validate(const Order& order, std::string& reason) const override
    {
        for (const auto& v : mChain)
        {
            if (!v->validate(order, reason))
            {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<IValidator>> mChain; // List of validators
};

class NoOpValidator final : public IValidator {
public:
    bool validate(const Order&, std::string&) const override { return true; }
};

#endif // ORDER_VALIDATOR_H
