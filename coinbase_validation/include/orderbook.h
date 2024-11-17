#pragma once
// Header Guard

#include "dro/flat-rb-tree.hpp"

#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

namespace gkp {

struct OrderBookLevel {
  double quantity_{};
  OrderBookLevel() = default;

  explicit OrderBookLevel(double quantity) : quantity_(quantity) {}
};

class LimitOrderBook {
 public:
  using price_level  = double;
  // Custom flat map, with size optimizations
  using bid_flat_map = dro::FlatMap<price_level, OrderBookLevel, uint32_t,
                                    std::greater<price_level>>;
  using ask_flat_map = dro::FlatMap<price_level, OrderBookLevel, uint32_t,
                                    std::less<price_level>>;

 private:
  constexpr static double epsilon = 1e-9;
  constexpr static uint16_t initialSize{500};
  bid_flat_map bid_{initialSize};
  ask_flat_map ask_{initialSize};
  std::string productID_;

 public:
  LimitOrderBook() = default;

  // productID used for printing
  explicit LimitOrderBook(std::string productID)
      : productID_(std::move(productID))
  {}

  void buildSides(const bool buySell, const double price, const double quantity)
  {
    if (buySell) {
      bid_.emplace(price, OrderBookLevel{quantity});
      return;
    }
    ask_.emplace(price, OrderBookLevel{quantity});
  }

  void updateBook(const char buySell, const double price, const double quantity)
  {
    if (buySell == 'b') {
      const auto it =
          bid_.emplace(price).first;  // default value built in place
      it->second.quantity_ = quantity;
      if (quantity < epsilon) {
        bid_.erase(it);
      }
      return;
    }
    const auto it = ask_.emplace(price).first;  // default value built in place
    it->second.quantity_ = quantity;
    if (quantity < epsilon) {
      ask_.erase(it);
    }
  }

  void printLevels(const uint16_t depth) const
  {
    time_t now = time(nullptr);
    std::cout << "\nTime: " << ctime(&now) << "Limit Orderbook: " << productID_
              << "\nAsk Levels:\n";

    int32_t count{};
    auto askEnd = ask_.end();
    auto it     = ask_.begin();
    for (; count < depth - 1 && it != askEnd; ++count, ++it) {
    }
    for (; count >= 0 && it != askEnd; --count, --it) {
      std::cout << std::fixed << std::setprecision(2) << "Level " << count + 1
                << " - Price: " << it->first;
      std::cout << std::fixed << std::setprecision(8)
                << ", Quantity: " << it->second.quantity_ << '\n';
    }

    std::cout << "Bid Levels:\n";
    count       = 0;
    auto bidEnd = bid_.end();
    for (auto it = bid_.begin(); count < depth && it != bidEnd; ++count, ++it) {
      std::cout << std::fixed << std::setprecision(2) << "Level " << count + 1
                << " - Price: " << it->first;
      std::cout << std::fixed << std::setprecision(8)
                << ", Quantity: " << it->second.quantity_ << '\n';
    }
  }

  void clearBook()
  {
    bid_.clear();
    ask_.clear();
  }

  [[nodiscard]] bool isCrossed() const
  {
    if (ask_.empty() || bid_.empty()) {
      return false;
    }
    return ask_.begin()->first <= bid_.begin()->first;
  }
};
}  // namespace gkp
