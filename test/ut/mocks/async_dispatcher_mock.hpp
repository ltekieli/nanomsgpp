#ifndef ASYNC_DISPATCHER_MOCK_HPP_
#define ASYNC_DISPATCHER_MOCK_HPP_

#include <errno.h>
#include <functional>
#include <gmock/gmock.h>

#include <boost/asio.hpp>

struct async_dispatcher_mock
{
  async_dispatcher_mock(int receive_sock, int send_sock,
                        boost::asio::io_service&)
      : m_receive_sock(receive_sock), m_send_sock(send_sock)
  {
  }

  using handler = std::function<void(const std::error_code&)>;
  MOCK_CONST_METHOD1(on_receive_event, void(handler));
  MOCK_CONST_METHOD1(on_send_event, void(handler));

  int m_receive_sock;
  int m_send_sock;
};

#endif // ASYNC_DISPATCHER_MOCK_HPP_
