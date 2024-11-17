#pragma once
// Header Guard

#include "common.h"
#include "dro/oa-hashmap.hpp"
#include "fast-double-parser/fast_double_parser.h"
#include "glaze/glaze.hpp"
#include "message_types.h"
#include "orderbook.h"
#include "websocket.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

namespace ssl = boost::asio::ssl;

namespace gkp {

class MessageParser {
 private:
  using side_type = std::vector<std::array<std::string, 2>>;

  SubscribeMsg subMessage_;
  std::vector<LimitOrderBook> orderbooksStorage_;
  dro::HashMap<std::string, uint16_t> productOrderbookID_{""};

  dro::HashMap<std::string, std::vector<std::size_t>> orderbookTimes_{""};
  dro::HashMap<std::string, std::size_t> snapshotTotals_{""};
  dro::HashMap<std::string, std::vector<std::size_t>> l2updateTotals_{""};

  std::shared_ptr<Websocket> websocket_ = nullptr;
  io_context& ioc_;
  ssl_context& ctx_;

 public:
  explicit MessageParser(const SubscribeMsg& sub, io_context& ioc,
                         ssl_context& ctx)
      : subMessage_(sub), ioc_(ioc), ctx_(ctx)
  {
    productOrderbookID_.reserve(sub.product_ids.size());
  }

  ~MessageParser()                               = default;
  // Non-copyable & Non-moveable
  MessageParser(const MessageParser&)            = delete;
  MessageParser& operator=(const MessageParser&) = delete;
  MessageParser(MessageParser&&)                 = delete;
  MessageParser& operator=(MessageParser&&)      = delete;

  void subscribeToMarketData()
  {
    std::string message = glz::write_json(subMessage_).value_or("error");

    websocket_          = std::make_shared<Websocket>(
        ioc_, ctx_, message,
        [this](std::string& json) { level2EventHandler(json); });
    websocket_->run();
  }

  void level2EventHandler(const std::string& json)
  {
    struct MsgType {
      std::string type;
    } msg;

    if (!glz::read_json<MsgType>(msg, json)) {
      return;  // Error handling omitted
    }

    if (msg.type == "l2update") {
      if (!parseL2Update(json)) {
        return;  // Error handling omitted
      }
    } else if (msg.type == "snapshot") {
      if (!parseSnapshot(json)) {
        return;  // Error handling omitted
      }
    } else if (msg.type == "subscriptions") [[unlikely]] {
      if (!MessageParser::validateSubscribeRecv(json)) {
        return;  // Error handling omitted
      }
    } else {
      return;  // Error Fallthrough
    }
  }

  void printOrderbookWithStats(const uint16_t depth)
  {
    for (const auto& orderbook : orderbooksStorage_) {
      orderbook.printLevels(depth);
    }

    std::cout << "\nStatistics:";

    if (!snapshotTotals_.empty()) {
      std::cout << "\nTotal Snapshot Processing Time:\n";
    }

    for (const auto& snapTime : snapshotTotals_) {
      std::cout << snapTime.first << " - Time: " << snapTime.second << " ns\n";
    }

    if (!l2updateTotals_.empty()) {
    std::cout << "\nL2 Message Processing Time:";
    }
    for (auto& l2update : l2updateTotals_) {
      std::cout << '\n' << l2update.first << '\n';
      MessageParser::analyzeData(l2update.second);
    }

    if (!orderbookTimes_.empty()) {
    std::cout << "\nOrderbook Update Time:";
    }
    for (auto& orderbookTime : orderbookTimes_) {
      std::cout << '\n' << orderbookTime.first << '\n';
      MessageParser::analyzeData(orderbookTime.second);
    }

    snapshotTotals_.clear();
    l2updateTotals_.clear();
    orderbookTimes_.clear();
  }

 private:

  [[nodiscard]] bool parseSnapshot(const std::string& json)
  {
    auto startSnap = std::chrono::high_resolution_clock::now();
    bool success{};

    SnapshotMsg snapshot;
    success                 = glz::read_json<SnapshotMsg>(snapshot, json);

    std::string& product_id = snapshot.product_id;
    auto iter               = productOrderbookID_.find(product_id);
    // If the product doesn't exist, add to hashmap and build book
    if (iter == productOrderbookID_.end()) {
      iter = productOrderbookID_.emplace(product_id, orderbooksStorage_.size())
                 .first;
      orderbooksStorage_.emplace_back(product_id);
    }
    uint16_t bookID = iter->second;
    auto& orderbook = orderbooksStorage_[bookID];
    orderbook.clearBook();

    updateOrderbookSnap(product_id, true, snapshot.bids, orderbook);
    updateOrderbookSnap(product_id, false, snapshot.asks, orderbook);

    auto endSnap = std::chrono::high_resolution_clock::now();
    snapshotTotals_[product_id] =
        std::chrono::duration_cast<std::chrono::nanoseconds>(endSnap
                                                             - startSnap)
            .count();
    return success;
  }

  void updateOrderbookSnap(const std::string& product_id, const bool buySell,
                           const side_type& side, auto& orderbook)
  {
    std::vector<std::size_t>& orderbookTimes = orderbookTimes_[product_id];

    double price{};
    double quantity{};
    for (const auto& level : side) {
      // Assume Successful parse
      const char* valid =
          fast_double_parser::parse_number(level[0].data(), &price);
      valid      = fast_double_parser::parse_number(level[1].data(), &quantity);

      auto start = std::chrono::high_resolution_clock::now();
      orderbook.buildSides(buySell, price, quantity);
      auto end = std::chrono::high_resolution_clock::now();

      orderbookTimes.emplace_back(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count());
    }
  }

  [[nodiscard]] bool parseL2Update(const std::string& json)
  {
    auto startL2 = std::chrono::high_resolution_clock::now();
    bool success{};

    L2UpdateMsg l2update;
    success                 = glz::read_json<L2UpdateMsg>(l2update, json);

    std::string& product_id = l2update.product_id;
    auto iter               = productOrderbookID_.find(product_id);
    if (iter == productOrderbookID_.end()) {
      return false; 
    }

    uint16_t bookID = iter->second;
    auto& orderbook = orderbooksStorage_[bookID];
    updateOrderbookL2(product_id, l2update, orderbook);

    auto endL2 = std::chrono::high_resolution_clock::now();
    l2updateTotals_[product_id].emplace_back(
        std::chrono::duration_cast<std::chrono::nanoseconds>(endL2 - startL2)
            .count());

    if (orderbook.isCrossed()) {
      restartWebsocket();
    }
    return success;
  }

  void updateOrderbookL2(const std::string& product_id,
                         const L2UpdateMsg& l2update, auto& orderbook)
  {
    std::vector<std::size_t>& orderbookTimes = orderbookTimes_[product_id];

    double price{};
    double quantity{};
    for (const auto& changes : l2update.changes) {
      // Assume Successful parse
      const char* valid =
          fast_double_parser::parse_number(changes[1].data(), &price);
      valid = fast_double_parser::parse_number(changes[2].data(), &quantity);

      auto start = std::chrono::high_resolution_clock::now();
      orderbook.updateBook(changes[0][0], price, quantity);
      auto end = std::chrono::high_resolution_clock::now();

      orderbookTimes.emplace_back(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count());
    }
  }

  [[nodiscard]] static bool validateSubscribeRecv(const std::string& json)
  {
    // Could do more work here to validate, but lets assume it's successful
    SubscribeRecv subRecv;
    return glz::read_json<SubscribeRecv>(subRecv, json);
  }

  static void analyzeData(std::vector<std::size_t>& data)
  {
    std::sort(data.begin(), data.end());
    auto mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    auto min  = data.empty() ? 0 : data.front();
    auto max  = data.empty() ? 0 : data.back();
    auto p99  = data.empty() ? 0 : data[0.99 * data.size()];

    std::cout << "Count:     " << data.size() << "\n";
    std::cout << "Min (ns):  " << min << '\n';
    std::cout << "Mean (ns): " << mean << '\n';
    std::cout << "99% (ns):  " << p99 << '\n';
    std::cout << "Max (ns):  " << max << '\n';
  }

  void restartWebsocket()
  {
    websocket_->restart();
    websocket_.reset();
    subscribeToMarketData();
  }
};

}  // namespace gkp
