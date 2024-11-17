#include "common.h"
#include "glaze/glaze.hpp"
#include "loop.h"
#include "message_parser.h"
#include "message_types.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <cstddef>

void
printUserSelection(const gkp::SubscribeMsg& sub)
{
  std::cout << "User Selection:\nProduct IDs:\n";
  for (const auto& symbol : sub.product_ids) {
    std::cout << "- " << symbol << '\n';
  }
  std::cout << "\nPress Ctrl+C to stop.\n";
}

int
main(int argc, char* argv[])
{
  constexpr auto helpOpt     = "help";
  constexpr auto productsOpt = "products";

  std::string productsDefault{"BTC-USD,ETH-USD"};

  namespace progOpt = boost::program_options;
  progOpt::options_description desc("Options");
  desc.add_options()(helpOpt, "Print help message.")(
      productsOpt,
      progOpt::value<std::string>()->default_value(productsDefault),
      "Products IDs, comma separated.");

  progOpt::variables_map varsMap;
  progOpt::store(progOpt::parse_command_line(argc, argv, desc), varsMap);

  // Print Help & End Program
  if (varsMap.count(helpOpt)) {
    std::cout << "./market-data-demo:\n" << desc << '\n';
    return 0;
  }

  // Parse user input, and print to terminal
  gkp::SubscribeMsg sub;
  boost::split(sub.product_ids, varsMap[productsOpt].as<std::string>(),
               boost::is_any_of(","), boost::token_compress_on);
  printUserSelection(sub);

  // Initialize IO Context and SSL
  namespace ssl = boost::asio::ssl;
  io_context ioc;
  ssl_context ctx{ssl::context::tlsv12_client};
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);

  // Main Class
  gkp::MessageParser parser{sub, ioc, ctx};

  // Set Loop Callback
  constexpr uint16_t depth = 5;
  const std::chrono::seconds intervalTime{5};
  gkp::Loop loop{ioc, intervalTime, [&parser, &depth]() mutable {
                   parser.printOrderbookWithStats(depth);
                 }};

  // Start DataStream
  parser.subscribeToMarketData();
  ioc.run();
}
