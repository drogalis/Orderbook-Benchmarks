#pragma once

// Header Guard

namespace gkp {

struct OrderBookLevel {
  double quantity_{};
  OrderBookLevel() = default;

  explicit OrderBookLevel(const double& quantity) : quantity_(quantity) {}
};

}  // namespace gkp
