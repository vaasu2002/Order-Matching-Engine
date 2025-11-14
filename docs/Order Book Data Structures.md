# Order Book Data Structures and In-Memory Layout

| Operation              | What Happens                                    | Structure Touched | Time              |
| ---------------------- | ----------------------------------------------- | ----------------- | ----------------- |
| **Add Order**          | Insert/find price level → append to linked list | Map + Linked List | `O(log P) + O(1)` |
| **Cancel Order**       | Find cached iterator → remove node              | Linked List       | `O(1)`            |
| **Match Orders**       | Walk top price levels, consume head orders      | Map + Linked List | `O(D + N)`        |
| **Remove Empty Level** | Erase key from map if list empty                | Map               | `O(log P)`        |
| **Best Bid/Ask**       | Access `begin()`/`rbegin()`                     | Map               | `O(1)`            |


## Linked List (used inside each PriceLevel)
Each price level holds a queue of orders at the same price — FIFO order.
```
PriceLevel (price = 100.50)
┌────────────────────────────────────────────┐
│ Head → [Order#1] → [Order#2] → [Order#3] → NULL
└────────────────────────────────────────────┘
Each [Order] node:
┌───────────────────────────┐
│ orderId | qty | nextPtr   │
└───────────────────────────┘
```

## Ordered Map (used in OrderTracker)
OrderTracker stores PriceLevels keyed by price in an ordered map (like std::map or std::map<Price, PriceLevelPtr>).
```
OrderTracker (for Bids)
───────────────────────────────────────────────
Ordered Map (price → PriceLevel)
───────────────────────────────────────────────
Price   | Value (pointer to PriceLevel)
───────────────────────────────────────────────
105.00  | → [PriceLevel@105.00]
104.50  | → [PriceLevel@104.50]
104.00  | → [PriceLevel@104.00]
───────────────────────────────────────────────
```

## Full System Diagram

```
                       ┌──────────────────────────────┐
                       │        OrderTracker          │
                       │ (one side: BID or ASK)       │
                       │──────────────────────────────│
                       │ Ordered Map<Price, LevelPtr> │
                       └──────────────┬───────────────┘
                                      │
      ┌───────────────────────────────┼───────────────────────────────┐
      │                               │                               │
┌─────────────────┐           ┌─────────────────┐           ┌─────────────────┐
│PriceLevel@105.00│           │PriceLevel@104.50│           │PriceLevel@104.00│
└───────┬─────────┘           └───────┬─────────┘           └───────┬─────────┘
        │                             │                             │
        ▼                             ▼                             ▼
 [Order#A]→[Order#B]→[Order#C]   [Order#D]→[Order#E]        [Order#F]→NULL
 FIFO linked list per price     FIFO linked list per price  FIFO linked list per price
```


