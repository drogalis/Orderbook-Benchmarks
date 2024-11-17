#pragma once
// Header Guard

#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/asio/ssl.hpp>

#include <chrono>

using Clock       = std::chrono::steady_clock;
using Duration    = std::chrono::nanoseconds;

using Timer       = boost::asio::basic_waitable_timer<Clock>;
using io_context  = boost::asio::io_context;
using ssl_context = boost::asio::ssl::context;
