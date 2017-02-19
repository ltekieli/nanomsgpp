#ifndef NATIVE_SOCKET_MOCK_HPP_
#define NATIVE_SOCKET_MOCK_HPP_

#include <boost/asio.hpp>
#include <functional>
#include <gmock/gmock.h>
#include <nmpp/native_socket.hpp>

struct native_socket_mock
{
  using native_handle_type = nmpp::native_socket::native_handle_type;

  native_socket_mock(boost::asio::io_service&,
                     nmpp::native_socket::native_handle_type sock)
      : m_sock(sock)
  {
  }

  using handler =
      std::function<void(const boost::system::error_code&, std::size_t)>;
  MOCK_CONST_METHOD1(async_read_event, void(handler));
  MOCK_CONST_METHOD1(async_write_event, void(handler));

  nmpp::native_socket::native_handle_type m_sock;
};

#endif // NATIVE_SOCKET_MOCK_HPP_
