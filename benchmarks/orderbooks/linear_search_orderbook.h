#pragma once
// Header Guard

#include "helper/orderbook_level.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace gkp {

class LinearSearchOrderbook {
 public:
  using price_level   = double;
  using pair_type = std::pair<price_level, OrderBookLevel>;
  using bid_container = std::vector<pair_type>;
  using ask_container = std::vector<pair_type>;

 private:
  constexpr static double epsilon = 1e-9;
  bid_container bid_;
  ask_container ask_;

 public:
  LinearSearchOrderbook() = default;

  void build_sides(const char buy_sell, const double& price,
                   const double& quantity)
  {
    if (buy_sell == 'b') {
      bid_.emplace_back(price, OrderBookLevel{quantity});
      return;
    }
    ask_.emplace_back(price, OrderBookLevel{quantity});
  }

  void update_book(const char buy_sell, const double& price,
                   const double& quantity)
  {
    // Bid ///////////////
    if (buy_sell == 'b') {
      auto it = std::find_if(bid_.begin(), bid_.end(), [&price](const pair_type& pair){ return pair.first == price; });
      if (it != bid_.end()) {
        // Erase
        if (quantity < epsilon) {
          bid_.erase(it);
        } else {
          // Update
          it->second.quantity_ = quantity;
        }
        // Insert
      } else if (quantity >= epsilon) {
        bid_.emplace_back(price, OrderBookLevel{quantity});
      }
      return;
    }
    // Ask ///////////////
    auto it = std::find_if(ask_.begin(), ask_.end(),[&price](const pair_type& pair){ return pair.first == price; });
    if (it != ask_.end()) {
      // Erase
      if (quantity < epsilon) {
        ask_.erase(it);
      } else {
        // Update
        it->second.quantity_ = quantity;
      }
      // Insert
    } else if (quantity >= epsilon) {
      ask_.emplace_back(price, OrderBookLevel{quantity});
    }
  }

  void clear_book()
  {
    bid_.clear();
    ask_.clear();
  }

  // Do not use, the first and last iterators need to be cached, TBD
  [[nodiscard]] bool is_crossed() const
  {
    if (ask_.empty() || bid_.empty()) {
      return false;
    }
    return ask_.begin()->first <= bid_.begin()->first;
  }
};
}  // namespace gkp
