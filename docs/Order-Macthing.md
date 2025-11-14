# Order Matching Logic

This document describes the core order-matching logic used in this engine, the main data structures, operation-level time complexities, and rationale for the complexity bounds. The notes are written to match the implementation style used across the codebase: per-price FIFO price levels, an ordered map of price levels, and cached per-order iterators for O(1) cancellations.

| Operation | Description | Time Complexity |
| --- | --- | --- |
| **Add order** | Insert price level if necessary, append order to FIFO queue | **O(log P)** to find/insert price level + **O(1)** to append |
| **Cancel (remove) order** | Use cached iterator to find price level + remove from list | **O(1)** (no search needed) |
| **Match orders (market order)** | Repeatedly remove from best price levels | **O(D + N)** where **D = number of price levels touched**, **N = number of orders consumed** |
| **Remove empty price level** | After final order removed | **O(log P)** |
| **Get best bid/ask** | From ordered map | **O(1)** (if map supports begin()/rbegin()) or **O(log P)** depending on structure |

Legend:
- `P` — number of active price levels (a price level is active if it contains ≥1 order)
- `D` — depth: number of price levels examined during a match
- `N` — number of individual orders matched / consumed

**Key Concepts**

- **PriceLevel**: represents all orders at a single price. Orders inside a price level are stored in FIFO order (price–time priority). A linked list or deque is typically used so that appends and removals at arbitrary cached positions are O(1).
- **OrderTracker**: maintains the set of `PriceLevel`s for one book side (bids or asks). Price levels are stored in an ordered map keyed by price so the engine can access the best price quickly and iterate price-by-price when matching. Each order also stores a cached pointer/iterator to its node inside its `PriceLevel` and a reference to the `PriceLevel` itself to support O(1) cancellation/removal.

**Data structure choices & rationale**

- Ordered map keyed by price (balanced BST or similar): enables efficient insertion/removal of price levels in O(log P) and fast access to the best price (begin()/rbegin()).
- Each `PriceLevel` contains a linked list (or intrusive list) of orders to preserve FIFO and allow O(1) removal given an iterator.
- Per-order metadata caches: the order holds a pointer to its `PriceLevel` and an iterator/node handle inside that level — this removes the need to search when cancelling.

Operation details

- Add order
	- Find the `PriceLevel` for the order's price in the `OrderTracker` (O(log P)). If it doesn't exist, insert a new `PriceLevel` (O(log P)).
	- Append the order to the `PriceLevel`'s FIFO list (O(1)).
	- Store the iterator/pointer to the appended node inside the order metadata for future O(1) cancellation.

- Cancel (remove) order
	- Look up the order's cached `PriceLevel` and iterator — remove the node from the price level's list (O(1)).
	- If the `PriceLevel` becomes empty, remove it from the ordered map (O(log P)).

- Match orders (market or aggressive limit)
	- Start at the best price for the resting side (best bid for sells; best ask for buys).
	- For each price level touched (this accounts for `D`), iterate its FIFO list consuming orders until the incoming quantity is satisfied or the price level is exhausted.
	- Each matched order requires O(1) work: adjust quantities, generate fills, remove or update the resting order node.
	- When a `PriceLevel` becomes empty, remove it from the ordered map (O(log P)).
	- Total complexity: O(D + N) plus O(log P) costs for any price-level insertions/removals during the process. The O(D + N) term is dominant for large matches where many orders are consumed.

Why O(D + N) is optimal

To correctly produce per-order fills and maintain book state, any engine must:
- Inspect each price level it considers (D), because whether to continue scanning depends on the state at that price (presence/quantity of orders).
- Touch each individual matched order (N) to produce fills, update balances, and remove/update the order.

Hence Ω(D + N) is a lower bound for any correct per-order matching engine that emits per-order fills and maintains a detailed book.

Pseudocode: market match (high level)

```pseudo
function matchMarket(incomingQty, restingSideOrderTracker):
	remaining = incomingQty
	while remaining > 0 and restingSideOrderTracker.hasBestPrice():
		level = restingSideOrderTracker.bestPriceLevel()
		while remaining > 0 and level.hasOrders():
			resting = level.frontOrder()  # earliest order at this price
			matched = min(remaining, resting.remainingQty)
			produceFill(incoming, resting, matched)
			remaining -= matched
			resting.remainingQty -= matched
			if resting.remainingQty == 0:
				removeOrderFromLevel(resting, level)  # O(1) via cached iterator
		if not level.hasOrders():
			restingSideOrderTracker.removePriceLevel(level)  # O(log P)
	return fills
```

Implementation notes and considerations

- Concurrency: if the engine is multi-threaded, access to the ordered map and individual price-level lists must be synchronized. Fine-grained locking per price level (or lock-free data structures) can reduce contention.
- Atomicity & events: matching should produce deterministic fills and publish events (fills, trades, order updates) only after state changes are atomically applied.
- Memory & locality: using an intrusive linked list for orders (order nodes embedded in order objects) reduces allocations and improves locality.
- Latency engineering: although asymptotic complexity is fixed, real systems optimize constants (preallocated pools, compact order representations, batching notifications, avoiding system calls on fast paths).

Cross references

- See `OrderBook/PriceLevel.h` and `OrderTracker/OrderTracker.h` for the code-level representations of price levels and trackers in this repo.

Summary

The engine uses an ordered map of FIFO `PriceLevel`s plus per-order cached iterators to achieve O(log P) price-level operations and O(1) per-order cancellations. Market matching runs in O(D + N) time, which is essentially optimal because the algorithm must inspect each price level it touches and each matched order it produces.

