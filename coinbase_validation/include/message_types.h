#pragma once
// Header Guard

#include <array>
#include <string>
#include <vector>

namespace gkp {

struct SubscribeMsg {
  std::string type = "subscribe";
  std::vector<std::string> product_ids;
  std::array<std::string, 1> channels{"level2_batch"};
};

struct Channels {
  std::string name;
  std::vector<std::string> product_ids;
  std::string account_ids;
};

struct SubscribeRecv {
  std::string type;
  std::vector<Channels> channels;
};

struct SnapshotMsg {
  std::string type;
  std::string product_id;
  std::vector<std::array<std::string, 2>> bids;
  std::vector<std::array<std::string, 2>> asks;
};

struct L2UpdateMsg {
  std::string type;
  std::string product_id;
  std::vector<std::array<std::string, 3>> changes;
  std::string time;
};
}  // namespace gkp
