#include "order_book.hpp"

#include <iostream>

namespace {

bool crosses(Side side, Price incoming, Price resting) {
    return side == Side::Buy ? incoming >= resting : incoming <= resting;
}

}

template <typename Book>
std::vector<Trade> OrderBook::match(Order& incoming, Book& opposite) {
    std::vector<Trade> trades;

    while (incoming.quantity > 0 && !opposite.empty()) {
        auto level = opposite.begin();
        Price level_price = level->first;

        if (incoming.type == OrderType::Limit &&
            !crosses(incoming.side, incoming.price, level_price)) {
            break;
        }

        Level& queue = level->second;
        while (incoming.quantity > 0 && !queue.empty()) {
            Order& resting = queue.front();
            Quantity fill = std::min(incoming.quantity, resting.quantity);

            OrderId buy_id = incoming.side == Side::Buy ? incoming.id : resting.id;
            OrderId sell_id = incoming.side == Side::Buy ? resting.id : incoming.id;
            trades.push_back({buy_id, sell_id, level_price, fill});

            incoming.quantity -= fill;
            resting.quantity -= fill;

            if (resting.quantity == 0) {
                index_.erase(resting.id);
                queue.pop_front();
            }
        }

        if (queue.empty()) {
            opposite.erase(level);
        }
    }

    return trades;
}

template <typename Book>
void OrderBook::rest(Order order, Book& book) {
    Level& queue = book[order.price];
    queue.push_back(order);
    auto it = std::prev(queue.end());
    index_[order.id] = {order.side, order.price, it};
}

std::vector<Trade> OrderBook::add(Order order) {
    std::vector<Trade> trades;

    if (order.side == Side::Buy) {
        trades = match(order, asks_);
        if (order.quantity > 0 && order.type == OrderType::Limit) {
            rest(order, bids_);
        }
    } else {
        trades = match(order, bids_);
        if (order.quantity > 0 && order.type == OrderType::Limit) {
            rest(order, asks_);
        }
    }

    return trades;
}

bool OrderBook::cancel(OrderId id) {
    auto found = index_.find(id);
    if (found == index_.end()) {
        return false;
    }

    const Location& loc = found->second;
    if (loc.side == Side::Buy) {
        auto level = bids_.find(loc.price);
        level->second.erase(loc.it);
        if (level->second.empty()) {
            bids_.erase(level);
        }
    } else {
        auto level = asks_.find(loc.price);
        level->second.erase(loc.it);
        if (level->second.empty()) {
            asks_.erase(level);
        }
    }

    index_.erase(found);
    return true;
}

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

void OrderBook::print(std::size_t depth) const {
    auto level_qty = [](const Level& q) {
        Quantity total = 0;
        for (const auto& o : q) total += o.quantity;
        return total;
    };

    std::cout << "        BIDS            ASKS\n";
    auto bid_it = bids_.begin();
    auto ask_it = asks_.begin();

    for (std::size_t i = 0; i < depth; ++i) {
        std::string bid_str = "     .        ";
        std::string ask_str = "     .";

        if (bid_it != bids_.end()) {
            bid_str = std::to_string(level_qty(bid_it->second)) +
                      " @ " + std::to_string(bid_it->first);
            bid_str.resize(14, ' ');
            ++bid_it;
        }
        if (ask_it != asks_.end()) {
            ask_str = std::to_string(level_qty(ask_it->second)) +
                      " @ " + std::to_string(ask_it->first);
            ++ask_it;
        }

        std::cout << "  " << bid_str << "  " << ask_str << "\n";
    }
    std::cout << std::endl;
}

template std::vector<Trade> OrderBook::match<OrderBook::BidBook>(Order&, BidBook&);
template std::vector<Trade> OrderBook::match<OrderBook::AskBook>(Order&, AskBook&);
template void OrderBook::rest<OrderBook::BidBook>(Order, BidBook&);
template void OrderBook::rest<OrderBook::AskBook>(Order, AskBook&);
