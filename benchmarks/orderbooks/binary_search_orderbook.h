#pragma once
// Header Guard

#include "helper/orderbook_level.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace gkp {

class BinarySearchOrderbook {
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
  BinarySearchOrderbook() = default;

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
      auto it = std::lower_bound(bid_.begin(), bid_.end(), price, [](const pair_type& pair, const double& price){ return pair.first < price; });
      if (it != bid_.end() && it->first == price) {
        // Erase
        if (quantity < epsilon) {
          bid_.erase(it);
          std::sort(bid_.begin(), bid_.end(), [](const pair_type& pairA, const pair_type& pairB){ return pairA.first < pairB.first; });
        } else {
          // Update
          it->second.quantity_ = quantity;
        }
        // Insert
      } else if (quantity >= epsilon) {
        bid_.emplace_back(price, OrderBookLevel{quantity});
        std::sort(bid_.begin(), bid_.end(), [](const pair_type& pairA, const pair_type& pairB){ return pairA.first < pairB.first; });
      }
      return;
    }
    // Ask ///////////////
    auto it = std::lower_bound(ask_.begin(), ask_.end(), price, [](const pair_type& pair, const double& price){ return pair.first < price; });
    if (it != ask_.end() && it->first == price) {
      // Erase
      if (quantity < epsilon) {
        ask_.erase(it);
        std::sort(ask_.begin(), ask_.end(), [](const pair_type& pairA, const pair_type& pairB){ return pairA.first < pairB.first; });
      } else {
        // Update
        it->second.quantity_ = quantity;
      }
      // Insert
    } else if (quantity >= epsilon) {
      ask_.emplace_back(price, OrderBookLevel{quantity});
      std::sort(ask_.begin(), ask_.end(), [](const pair_type& pairA, const pair_type& pairB){ return pairA.first < pairB.first; });
    }
  }

  void clear_book()
  {
    bid_.clear();
    ask_.clear();
  }

  [[nodiscard]] bool is_crossed() const
  {
    if (ask_.empty() || bid_.empty()) {
      return false;
    }
    return ask_.begin()->first <= bid_.rbegin()->first;
  }
};
}  // namespace gkp
