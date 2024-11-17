#pragma once
// Header Guard

#include "common.h"

#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/system/detail/error_code.hpp>

#include <cassert>
#include <chrono>
#include <functional>

namespace gkp {

struct Loop {
  using Callback = std::function<void()>;

  Loop(io_context& ioc, Duration d, Callback cb)
      : ioc_{ioc}, delay_{d}, callback_{std::move(cb)}, timer_{ioc}
  {
    register_event();
  }

 private:
  io_context& ioc_;
  Duration delay_;
  Callback callback_;
  Timer timer_;

  void register_event()
  {
    timer_.expires_after(delay_);
    timer_.async_wait(std::bind_front(&Loop::handle, this));
  }

  void handle(const boost::system::error_code& ec)
  {
    // for simplicity we assume the timer worked.
    assert(!ec.failed());

    callback_();
    register_event();
  }
};
}  // namespace gkp
