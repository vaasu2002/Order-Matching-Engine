//
// Created by Vaasu Bisht on 20/10/25.
//

#include "Validation.h"
#include "Order.h"

bool QuantityValidator::validate(const Order& order, std::string& reason) const
{
    if (order.qty() <= Quantity{0})
    {
        reason = "Quantity must be > 0";
        return false;
    }
    return true;
}

bool LimitPriceRequiredValidator::validate(const Order& order, std::string& reason) const
{
    if (order.type() == Type::LIMIT || order.type() == Type::STOP_LIMIT)
    {
        if (order.price() <= Price{0})
        {
            reason = "Limit/stop-limit requires limit price > 0";
            return false;
        }
    }
    return true;
}

bool StopPriceRequiredValidator::validate(const Order& order, std::string& reason) const
{
    if (order.type() == Type::STOP || order.type() == Type::STOP_LIMIT)
    {
        if (order.stopPrice() <= Price{0})
        {
            reason = "Stop/stop-limit requires stop price > 0";
            return false;
        }
    }
    return true;
}