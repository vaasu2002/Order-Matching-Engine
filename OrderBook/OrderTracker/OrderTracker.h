//
// Created by Vaasu Bisht on 20/10/25.
//

#ifndef ORDERTRACKER_H
#define ORDERTRACKER_H

#include "../PriceLevel/PriceLevel.h"
#include <map>

/**
 * @struct PriceComparator
 * @brief Custom comparator for price-based map ordering
 *
 * In the BUY side: High price has priority.
 * In the SELL side: Low price has priority.
 */
struct PriceComparator
{
 bool isBuySide;
 explicit PriceComparator(const bool isBuySide = false): isBuySide(isBuySide){}
 bool operator()(const Price a, const Price b) const
 {
  return isBuySide ? a>b : a<b;
 }
};

/**
 * @class OrderTracker
 * @brief Manages one side of the order book. Owns all the PriceLevel objects.
 */
class OrderTracker {
 using PriceLevelPtr = std::shared_ptr<PriceLevel>;
 using PriceLevelMap = std::map<Price, PriceLevelPtr, PriceComparator>;
 using OrderLocationMap = std::map<OrderId, std::pair<Price, typename PriceLevel::OrderIterator>>;

 Side mSide;
 OrderLocationMap mOrderLocationMap;
 PriceLevelMap mPriceLevelMap;

public:
 explicit OrderTracker(const Side side);

 /**
  * @brief Get the PriceLevel object for the given price.
  * @return Returns the PriceLevel if it exists, otherwise nullptr.
  */
 PriceLevelPtr getPriceLevel(Price price);

 /**
  * @brief Create a PriceLevel for the given price.
  * @warning This method does not check for existence. It unconditionally
  * creates a new PriceLevel. Prefer using ensurePriceLevel() instead.
  * It replaces if new instance inside PriceLevelMap, might lead to
  * losing information.
  *
  * @details
  * Made to follow SRP design principle and this method is used inside
  * ensurePriceLevel() to separate the PriceLevel creation logic.
  */
 PriceLevelPtr createPriceLevel(Price price);

 /**
  * @brief Ensure a PriceLevel exists for the given price. If it does not exist, a new one will be
  * created and returned.
  *
  * @remarks
  * It uses createPriceLevel() and uses it safely only after checking that PriceLevel of given price
  * does not exist.
  * @return
  */
 PriceLevelPtr getOrCreatePriceLevel(Price price);

 /**
  * @brief Add order to its respective PriceLevel.
  * @param order
  */
 void addOrder(OrderPtr order);

 /**
  * @brief Executes trades by matching an incoming order against the best-priced
  * resting orders.
  *
  * Consumes liquidity from the best-priced level on the opposite side of the book,
  * up to the specified quantity.
  * @param qty The remaining quantity of the incoming order to be filled.
  * @param min The minimum price at which BUY/SELL is allowed.
  * @param max The maximum price at which BUY/SELL is allowed.
  */
 void matchOrder(Quantity& qty, Price min, Price max);
};



#endif //ORDERTRACKER_H
