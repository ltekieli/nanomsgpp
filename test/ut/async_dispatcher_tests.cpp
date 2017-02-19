#include "mocks/native_socket_mock.hpp"
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <nmpp/async_dispatcher.hpp>

using namespace ::testing;

struct EventHandlerMock
{
  MOCK_METHOD1(handle, void(const std::error_code&));
};

struct async_dispatcher_tests : Test
{
  void SetUp()
  {
  }

  boost::asio::io_service io;
  EventHandlerMock handler;
};

TEST_F(async_dispatcher_tests, creates_native_socket_in_constructor)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(5, 6, io);
  ASSERT_THAT(async_dispatcher.get_native_receive_socket().m_sock, Eq(5));
}

TEST_F(async_dispatcher_tests, send_receive_native_sockets_are_different)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(5, 6, io);
  ASSERT_THAT(async_dispatcher.get_native_receive_socket().m_sock,
              Ne(async_dispatcher.get_native_send_socket().m_sock));
}

TEST_F(async_dispatcher_tests, throws_when_receive_on_invalid_socket)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(-1, 6, io);
  ASSERT_THROW(
      async_dispatcher.on_receive_event([](const std::error_code&) {}),
      std::logic_error);
}

TEST_F(async_dispatcher_tests, throws_when_send_on_invalid_socket)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(6, -1, io);
  ASSERT_THROW(async_dispatcher.on_send_event([](const std::error_code&) {}),
               std::logic_error);
}

TEST_F(async_dispatcher_tests, registers_read_handler)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(5, 6, io);
  auto& native_socket = async_dispatcher.get_native_receive_socket();
  native_socket_mock::handler native_handler;
  EXPECT_CALL(native_socket, async_read_event(_))
      .WillOnce(SaveArg<0>(&native_handler));
  async_dispatcher.on_receive_event(std::bind(
      &EventHandlerMock::handle, std::ref(handler), std::placeholders::_1));

  EXPECT_CALL(handler, handle(_));
  native_handler(boost::system::error_code(), 0);
}

TEST_F(async_dispatcher_tests, registers_write_handler)
{
  nmpp::async_dispatcher<native_socket_mock> async_dispatcher(5, 6, io);
  auto& native_socket = async_dispatcher.get_native_send_socket();
  native_socket_mock::handler native_handler;
  EXPECT_CALL(native_socket, async_write_event(_))
      .WillOnce(SaveArg<0>(&native_handler));
  async_dispatcher.on_send_event(std::bind(
      &EventHandlerMock::handle, std::ref(handler), std::placeholders::_1));

  EXPECT_CALL(handler, handle(_));
  native_handler(boost::system::error_code(), 0);
}
