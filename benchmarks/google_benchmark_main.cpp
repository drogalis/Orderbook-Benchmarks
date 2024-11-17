#include "benchmark/benchmark.h"
#include "orderbooks/binary_search_orderbook.h"
#include "orderbooks/boost_flat_map_orderbook.h"
#include "orderbooks/dro_flat_map_orderbook.h"
#include "orderbooks/linear_search_orderbook.h"
#include "orderbooks/std_map_ankerl_hashmap_orderbook.h"
#include "orderbooks/std_map_orderbook.h"
#include "sample_data_generator.hpp"

#include <cstdint>

static void
BM_stdMap_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<stdMapOrderbook> data{
      static_cast<size_t>(state.range(0))};
  stdMapOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

static void
BM_BoostFlatMap_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<BoostFlatMapOrderbook> data{
      static_cast<size_t>(state.range(0))};
  BoostFlatMapOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

static void
BM_stdMapAnkerl_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<stdMapAnkerlOrderbook> data{
      static_cast<size_t>(state.range(0))};
  stdMapAnkerlOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

static void
BM_LinearSearch_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<LinearSearchOrderbook> data{
      static_cast<size_t>(state.range(0))};
  LinearSearchOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

static void
BM_BinarySearch_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<BinarySearchOrderbook> data{
      static_cast<size_t>(state.range(0))};
  BinarySearchOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

static void
BM_DroFlatMap_Orderbook(benchmark::State& state)
{
  using namespace gkp;
  SampleDataGenerator<DroFlatMapOrderbook> data{
      static_cast<size_t>(state.range(0))};
  DroFlatMapOrderbook book;
  data.set_snapshot_price_levels(book);
  // run benchmark
  for (auto _ : state) {
    data.perform_sample_L2_messages(book);
  }
}

constexpr static uint32_t begin_size = 1<<7;
constexpr static uint32_t end_size   = 1<<16;
// Register the function as a benchmark
BENCHMARK(BM_stdMap_Orderbook)->RangeMultiplier(2)->Range(begin_size, end_size);
BENCHMARK(BM_BoostFlatMap_Orderbook)
    ->RangeMultiplier(2)
    ->Range(begin_size, end_size);
BENCHMARK(BM_stdMapAnkerl_Orderbook)
    ->RangeMultiplier(2)
    ->Range(begin_size, end_size);
//BENCHMARK(BM_LinearSearch_Orderbook)
//    ->RangeMultiplier(2)
//   ->Range(begin_size, end_size);
//BENCHMARK(BM_BinarySearch_Orderbook)
//    ->RangeMultiplier(2)
//    ->Range(begin_size, end_size);
BENCHMARK(BM_DroFlatMap_Orderbook)
    ->RangeMultiplier(2)
    ->Range(begin_size, end_size);

// Run the benchmark
BENCHMARK_MAIN();
