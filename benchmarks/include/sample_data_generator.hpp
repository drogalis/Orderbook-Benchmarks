#include <random>

namespace gkp {

template <typename Orderbook>
class SampleDataGenerator {
 private:
  // Change these user defined constants
  std::size_t LEVEL_QTY;
  constexpr static std::size_t ITERATIONS       = 10'000;
  // Denominator in a fraction e.g. 1/4. This is the probability of an erase
  // occurring. Default is 1/4 or 25%.
  constexpr static std::size_t DENOMINATOR      = 4;
  constexpr static std::size_t initial_best_ask = 100'001;
  constexpr static std::size_t initial_best_bid = 100'000;
  // Preset random devices
  std::minstd_rand generator{0};
  std::uniform_int_distribution<std::size_t> bid_uniform_distribution{
      initial_best_bid, initial_best_bid - LEVEL_QTY};
  std::uniform_int_distribution<std::size_t> ask_uniform_distribution{
      initial_best_ask, initial_best_ask + LEVEL_QTY};

  double get_random_price(const char& bid_ask)
  {
    if (bid_ask == 'b') {
      return static_cast<double>(bid_uniform_distribution(generator));
    }
    return static_cast<double>(ask_uniform_distribution(generator));
  }

  double get_random_quantity()
  {
    // The quantity doesn't matter so much, the important part is whether it's
    // zero or not
    return static_cast<double>(bid_uniform_distribution(generator)
                               % DENOMINATOR);
  }

  char get_random_buy_sell()
  {
    // I think a random 50/50 distribution makes sense
    return (bid_uniform_distribution(generator) & 1) ? 'b' : 's';
  }

 public:

  explicit SampleDataGenerator(const std::size_t level_qty = 1'000)
      : LEVEL_QTY(level_qty)
  {}

  void set_snapshot_price_levels(Orderbook& book)
  {
    // Bid ///////////
    for (std::size_t i{}, price = initial_best_bid; i < LEVEL_QTY; ++i, --price)
    {
      book.build_sides('b', price, 1.0);
    }
    // Ask ///////////
    for (std::size_t i{}, price = initial_best_ask; i < LEVEL_QTY; ++i, ++price)
    {
      book.build_sides('s', price, 1.0);
    }
  }

  void perform_sample_L2_messages(Orderbook& book)
  {
    for (std::size_t i{}; i < ITERATIONS; ++i) {
      const char buy_sell   = get_random_buy_sell();
      const double price    = get_random_price(buy_sell);
      const double quantity = get_random_quantity();
      book.update_book(buy_sell, price, quantity);
    }
  }
};

}
