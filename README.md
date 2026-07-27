# Limit Order Book

A single-threaded limit order book and matching engine in C++17, implementing
price-time priority for limit and market orders.

## Build

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/orderbook
```

Requires CMake 3.15+ and a C++17 compiler.

## What it does

- Accepts limit and market orders on both sides.
- Matches incoming orders against the opposite book, filling greedily across
  price levels until the order is exhausted or no crossing liquidity remains.
- Rests the unfilled remainder of a limit order; discards the remainder of a
  market order.
- Cancels resting orders by id.
- Emits a trade record for every fill.

## Design

Each side of the book is a `std::map` from price to a FIFO queue of orders.
Bids are keyed with `std::greater` and asks with `std::less`, so `begin()` is
always the best price on either side. Within a price level, orders sit in a
`std::list`, preserving arrival order for time priority and keeping iterators
stable so cancellation does not invalidate other resting orders.

An `unordered_map` from order id to its `{side, price, list iterator}` gives
constant-time cancellation without scanning the book.

## Scope and limitations

- Single-threaded; no concurrency or lock-free structures.
- No order modification; a change is a cancel followed by a new order.
- Prices are integer ticks, not floating point.
- No fees, no self-trade prevention, no iceberg or stop orders.
- The benchmark uses random uniform flow, which is not representative of real
  market microstructure and should be read only as a throughput sanity check.

## Next steps

- Replay a recorded message stream (ITCH or a CSV feed) instead of random flow.
- Add order modification and self-trade prevention.
- Swap the per-level `std::list` for an intrusive list to cut allocation.
