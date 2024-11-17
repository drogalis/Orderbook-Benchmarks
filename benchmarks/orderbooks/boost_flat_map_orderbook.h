#pragma once
// Header Guard

#include "helper/orderbook_level.hpp"

#if __has_include(<boost/container/flat_map.hpp>)
#include <boost/container/flat_map.hpp>
#endif

#include <cstdint>
#include <utility>

namespace gkp {

#if __has_include(<boost/container/flat_map.hpp>)
class BoostFlatMapOrderbook {
 public:
  using price_level   = double;
  using bid_container = boost::container::flat_map<price_level, OrderBookLevel,
                                                   std::greater<price_level>>;
  using ask_container = boost::container::flat_map<price_level, OrderBookLevel,
                                                   std::less<price_level>>;

 private:
  constexpr static double epsilon = 1e-9;
  bid_container bid_;
  ask_container ask_;

 public:
  BoostFlatMapOrderbook() = default;

  void build_sides(const char buy_sell, const double& price,
                   const double& quantity)
  {
    if (buy_sell == 'b') {
      bid_.emplace(price, OrderBookLevel{quantity});
      return;
    }
    ask_.emplace(price, OrderBookLevel{quantity});
  }

  void update_book(const char buy_sell, const double& price,
                   const double& quantity)
  {
    // Bid ///////////////
    if (buy_sell == 'b') {
      if (quantity < epsilon) {
        bid_.erase(price);
        return;
      }
      bid_.insert_or_assign(price, OrderBookLevel{quantity});
      return;
    }
    // Ask ///////////////
    if (quantity < epsilon) {
      ask_.erase(price);
      return;
    }
    ask_.insert_or_assign(price, OrderBookLevel{quantity});
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
    return ask_.begin()->first <= bid_.begin()->first;
  }
};

#else

// Empty Container
struct BoostFlatMapOrderbook {
  void build_sides(const bool buy_sell, const double& price,
                   const double& quantity)
  {}

  void update_book(const char buy_sell, const double& price,
                   const double& quantity)
  {}

  void clear_book() {}

  [[nodiscard]] bool is_crossed() const { return false; }
};
#endif

}  // namespace gkp
