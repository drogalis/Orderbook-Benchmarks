#pragma once
// Header Guard

#include "../../submodules/Dense-Hashmap/include/dro/dense_hashmap.hpp"
#include "helper/orderbook_level.hpp"

#include <cstdint>
#include <map>
#include <utility>

namespace gkp {

class stdMapDroOrderbook {
 public:
  using price_level = double;
  using bid_container =
      std::map<price_level, OrderBookLevel, std::greater<price_level>>;
  using ask_container =
      std::map<price_level, OrderBookLevel, std::less<price_level>>;

 private:
  constexpr static double epsilon = 1e-9;
  bid_container bid_;
  ask_container ask_;

  using bid_hashmap_t = dro::dense_hashmap<price_level, decltype(bid_.begin())>;
  using ask_hashmap_t = dro::dense_hashmap<price_level, decltype(ask_.begin())>;

  bid_hashmap_t bid_iterators_;
  ask_hashmap_t ask_iterators_;

 public:
  stdMapDroOrderbook() = default;

  void build_sides(const char buy_sell, const double& price,
                   const double& quantity)
  {
    if (buy_sell == 'b') {
      auto it = bid_.emplace(price, OrderBookLevel{quantity}).first;
      bid_iterators_.emplace(price, it);
      return;
    }
    auto it = ask_.emplace(price, OrderBookLevel{quantity}).first;
    ask_iterators_.emplace(price, it);
  }

  void update_book(const char buy_sell, const double& price,
                   const double& quantity)
  {
    // Bid ///////////////
    if (buy_sell == 'b') {
      auto find_it = bid_iterators_.find(price);
      if (quantity < epsilon) {
        if (find_it != bid_iterators_.end()) {
          bid_.erase(find_it->second);
          bid_iterators_.erase(find_it);
        }
        return;
      }
      if (find_it != bid_iterators_.end()) {
        find_it->second->second = OrderBookLevel{quantity};
        return;
      }
      auto it = bid_.emplace(price, OrderBookLevel{quantity}).first;
      bid_iterators_.emplace(price, it);
      return;
    }
    // Ask ///////////////
    auto find_it = ask_iterators_.find(price);
    if (quantity < epsilon) {
      if (find_it != ask_iterators_.end()) {
        ask_.erase(find_it->second);
        ask_iterators_.erase(find_it);
      }
      return;
    }
    if (find_it != ask_iterators_.end()) {
      find_it->second->second = OrderBookLevel{quantity};
      return;
    }
    auto it = ask_.emplace(price, OrderBookLevel{quantity}).first;
    ask_iterators_.emplace(price, it);
  }

  void clear_book()
  {
    bid_.clear();
    ask_.clear();
    bid_iterators_.clear();
    ask_iterators_.clear();
  }

  [[nodiscard]] bool is_crossed() const
  {
    if (ask_.empty() || bid_.empty()) {
      return false;
    }
    return ask_.begin()->first <= bid_.begin()->first;
  }
};
}  // namespace gkp
