#ifndef NMPP_NATIVE_SOCKET_HPP
#define NMPP_NATIVE_SOCKET_HPP

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace nmpp
{

class native_socket
{
  using socket_type = boost::asio::posix::stream_descriptor;

public:
  typedef socket_type::native_handle_type native_handle_type;

  native_socket(boost::asio::io_service& io, native_handle_type handle)
      : socket(io, handle)
  {
  }

  ~native_socket()
  {
    if (socket.is_open())
    {
      socket.release();
    }
  }

  template <typename handler_type>
  void async_read_event(handler_type&& handler)
  {
    socket.async_read_some(boost::asio::null_buffers(),
                           std::forward<handler_type>(handler));
  }

  template <typename handler_type>
  void async_write_event(handler_type&& handler)
  {
    socket.async_write_some(boost::asio::null_buffers(),
                            std::forward<handler_type>(handler));
  }

  native_handle_type native_handle()
  {
    return socket.native_handle();
  }

private:
  socket_type socket;
};

} // namespace aware

#endif // NMPP_NATIVE_SOCKET_HPP
