# Order Matching Engine


### Installation
```bash
# Install dependencies
chmod +x scripts/install-deps.sh
./scripts/install-deps.sh 

# Build and Run
cmake -S . -B build
cmake --build build
./build/OrderMatchingEngine 
````


### Order State
```mermaid
stateDiagram-v2
    [*] --> PENDING : Order Created

    PENDING --> FULFILLED : All quantity matched
    PENDING --> PARTIALLY_FILLED : Some quantity matched
    PENDING --> CANCELLED : Market order with no match

    PARTIALLY_FILLED --> FULFILLED : Remaining quantity matched
    PARTIALLY_FILLED --> CANCELLED : Cancel request (future)

    FULFILLED --> [*]
    CANCELLED --> [*]

    note right of PENDING
        Initial state after validation
        LIMIT orders remain in book
    end note

    note right of PARTIALLY_FILLED
        Part of order matched
        Remaining qty becomes resting order
    end note

    note right of FULFILLED
        Order completely matched
        Removed from order book
    end note

    note right of CANCELLED
        Market orders: No match available
        Future: Manual cancellation
    end note

```