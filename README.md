# Orderbook Benchmarks

## Usage

```
./build/market_data_demo --p BTC-USD,ETH-USD

./market-data-demo:
Options:
  --help                            Print help message.
  --products arg (=BTC-USD,ETH-USD) Products IDs, comma separated.
```

## Benchmarks

These benchmarks were taken on a (4) core Intel(R) Core(TM) i5-9300H CPU @ 2.40GHz with isolcpus on cores 2 and 3.
The linux kernel is v6.10.11-200.fc40.x86_64 and compiled with gcc version 14.2.1.

Most important aspects of benchmarking:
- Have at least one core isolated with isolcpus enabled in Grub.
- Compile with -DCMAKE_BUILD_TYPE=Release
- e.g. taskset -c 2 ./build/market_data_demo

```
File:
results/benchmark_results.txt
```

## Build Instructions

To build the executable, run the following commands:

```
    $ mkdir build
    $ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    $ cmake --build build
```
