#pragma once
// Header Guard

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace beast     = boost::beast;
namespace http      = beast::http;
namespace websocket = beast::websocket;
namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;

namespace gkp {

class Websocket : public std::enable_shared_from_this<Websocket> {
 private:
  using tcp             = boost::asio::ip::tcp;
  using parser_callback = std::function<void(std::string&)>;

  tcp::resolver resolver_;
  websocket::stream<ssl::stream<beast::tcp_stream>> ws_;
  beast::flat_buffer buffer_;

  std::string address_ = "ws-feed.exchange.coinbase.com";
  std::string port_    = "443";
  std::string message_;
  parser_callback parserCallback_;
  bool restart_{};

 public:
  explicit Websocket(net::io_context& ioc, ssl::context& ctx,
                     std::string message, parser_callback parserCallback)
      : resolver_(net::make_strand(ioc))
      , ws_(net::make_strand(ioc), ctx)
      , message_(std::move(message))
      , parserCallback_(std::move(parserCallback))
  {}

  // Start the asynchronous operation
  void run()
  {
    // Look up the domain name
    resolver_.async_resolve(
        address_, port_,
        beast::bind_front_handler(&Websocket::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
  {
    if (ec) {
      return fail(ec, "resolve");
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(&Websocket::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type ep)
  {
    if (ec) {
      return fail(ec, "connect");
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
                                  address_.c_str()))
    {
      ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                             net::error::get_ssl_category());
      return fail(ec, "connect");
    }

    address_ += ':' + std::to_string(ep.port());

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&Websocket::on_ssl_handshake,
                                  shared_from_this()));
  }

  void on_ssl_handshake(beast::error_code ec)
  {
    if (ec) {
      return fail(ec, "ssl_handshake");
    }

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(
        websocket::stream_base::decorator([](websocket::request_type& req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING)
                      + " websocket-client-async-ssl");
        }));

    // Perform the websocket handshake
    ws_.async_handshake(address_, "/",
                        beast::bind_front_handler(&Websocket::on_handshake,
                                                  shared_from_this()));
  }

  void on_handshake(beast::error_code ec)
  {
    if (ec) {
      return fail(ec, "handshake");
    }

    // Send the message
    ws_.async_write(
        net::buffer(message_),
        beast::bind_front_handler(&Websocket::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred)
  {
    if (ec) {
      return fail(ec, "write");
    }

    if (buffer_.size()) {
      auto bufData = buffer_.data();
      std::string json(buffers_begin(bufData), buffers_end(bufData));
      parserCallback_(json);
      buffer_.clear();
    }

    auto callback = (restart_) ? &Websocket::on_close : &Websocket::on_write;
    // Read a message into our buffer
    ws_.async_read(buffer_,
                   beast::bind_front_handler(callback, shared_from_this()));
  }

  void on_close(beast::error_code ec, std::size_t bytes_transferred)
  {
    if (ec) {
      return fail(ec, "close");
    }
    // Close the WebSocket connection
    ws_.async_close(
        websocket::close_code::normal,
        beast::bind_front_handler(&Websocket::finish, shared_from_this()));
  }

  void finish(beast::error_code ec)
  {
    if (ec) {
      return fail(ec, "finish");
    }
  }

  void restart() { restart_ = true; }

  void fail(beast::error_code ec, const char* what)
  {
    std::cerr << what << ": " << ec.message() << "\n";
  }
};

}  // namespace gkp
