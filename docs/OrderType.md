# Order Types:

1) Limit Order
- It is an instruction to buy/sell at a specific price or better.
- Example:
  -> A limit buy won't execute above 3,500
  -> A limit sell won't execute below 1,600
- If no matching counter-order is available at that price immediately, the
  limit order is stored in the order book, write for a future match.
- If there is a counter-order meeting the price → it executes immediately
  (fully or partially), and any unfilled remainder stays in the book.



2) Market Order (Match order or aggressive order)
- It is an instruction to buy/sell order immediately at the best available price.
- Engine looks at the top of the opposite side of the book and fills the order until:
    - The order quantity i fulfilled, or
    - The order book runs out of matching offers

| Property             | Limit Order                                | Market Order                               |
| -------------------- | -------------------------------------------|--------------------------------------------|
| Price specified      | Yes                                        | No                                         |
| Guaranteed execution | No                                         | Yes                                        |
| Stored in book       | Yes, if not filled                         | No                                         |
| Common use           | Placing bids/offers and providing liquidity| Taking liquidity, fast entry/exit          |