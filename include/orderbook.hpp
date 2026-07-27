#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class Side { Buy, Sell };
enum class OrderType { Limit, Market };

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;

struct Order {
    OrderId id;
    Side side;
    OrderType type;
    Price price;
    Quantity quantity;
};

struct Trade {
    OrderId buy_id;
    OrderId sell_id;
    Price price;
    Quantity quantity;
};

class OrderBook {
public:
    std::vector<Trade> add(Order order);
    bool cancel(OrderId id);

    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;

    std::size_t size() const { return index_.size(); }
    void print(std::size_t depth = 5) const;

private:
    using Level = std::list<Order>;
    using BidBook = std::map<Price, Level, std::greater<Price>>;
    using AskBook = std::map<Price, Level, std::less<Price>>;

    struct Location {
        Side side;
        Price price;
        Level::iterator it;
    };

    BidBook bids_;
    AskBook asks_;
    std::unordered_map<OrderId, Location> index_;

    template <typename Book>
    std::vector<Trade> match(Order& incoming, Book& opposite);

    template <typename Book>
    void rest(Order order, Book& book);
};
