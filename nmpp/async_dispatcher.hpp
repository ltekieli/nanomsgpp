#ifndef NMPP_ASYNC_DISPATCHER_HPP_
#define NMPP_ASYNC_DISPATCHER_HPP_

#include <boost/asio.hpp>
#include <nmpp/exception.hpp>

namespace nmpp
{

template <typename native_socket_type> class async_dispatcher
{
public:
  async_dispatcher(
      typename native_socket_type::native_handle_type receive_handle,
      typename native_socket_type::native_handle_type send_handle,
      boost::asio::io_service& io) noexcept
  {
    if (receive_handle != -1)
      receive_socket =
          std::make_unique<native_socket_type>(io, receive_handle);
    if (send_handle != -1)
      send_socket = std::make_unique<native_socket_type>(io, send_handle);
  }

  template <typename handler_type>
  void on_receive_event(handler_type&& handler) throw(std::logic_error)
  {
    throw_when<std::logic_error>(!receive_socket,
                                 "Receive operation not supported");

    receive_socket->async_read_event([handler =
                                          std::forward<handler_type>(handler)](
        const boost::system::error_code&, std::size_t) {
      handler(std::error_code());
    });
  }

  template <typename handler_type>
  void on_send_event(handler_type&& handler) throw(std::logic_error)
  {
    throw_when<std::logic_error>(!send_socket, "Send operation not supported");

    send_socket->async_write_event([handler =
                                        std::forward<handler_type>(handler)](
        const boost::system::error_code&, std::size_t) {
      handler(std::error_code());
    });
  }

  const native_socket_type& get_native_receive_socket()
  {
    return *receive_socket;
  }

  const native_socket_type& get_native_send_socket()
  {
    return *send_socket;
  }

private:
  std::unique_ptr<native_socket_type> receive_socket;
  std::unique_ptr<native_socket_type> send_socket;
};

} // namespace nmpp

#endif // NMPP_ASYNC_DISPATCHER_HPP_
