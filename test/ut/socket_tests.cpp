#include "mocks/async_dispatcher_mock.hpp"
#include "mocks/message_mock.hpp"
#include "mocks/nanomsg_mock.hpp"
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <nmpp/socket.hpp>
#include <type_traits>

using namespace ::testing;

struct socket_test : Test
{
  void SetUp()
  {
    EXPECT_CALL(nanomsg, nn_errno()).WillRepeatedly(Return(1));
  }

  int domain = 0;
  int proto = 0;
  nanomsg_mock nanomsg;
};

TEST_F(socket_test, creates_instance_with_given_domain_and_proto)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto));
  EXPECT_CALL(nanomsg, nn_close(_));
  nmpp::socket socket(domain, proto);
}

TEST_F(socket_test, throws_exception_when_nn_socket_returns_negative_value)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(-1));
  ASSERT_THROW(nmpp::socket socket(domain, proto), nmpp::exception);
}

TEST_F(socket_test, closes_socket)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
  nmpp::socket socket(domain, proto);
  EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(0));
  socket.close();
}

TEST_F(socket_test, throws_exception_when_closing_socket_returns_minus_one)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
  nmpp::socket socket(domain, proto);
  EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(-1));
  ASSERT_THROW(socket.close(), nmpp::exception);
}

TEST_F(socket_test,
       closes_socket_using_same_file_descriptor_as_given_during_creaion)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(100));
  nmpp::socket socket(domain, proto);
  EXPECT_CALL(nanomsg, nn_close(100)).WillOnce(Return(0));
  socket.close();
}

TEST_F(socket_test, closes_socket_on_destruction)
{
  InSequence seq;
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
  EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(0));

  {
    nmpp::socket socket(domain, proto);
  }
}

TEST_F(socket_test, does_not_throw_on_destruction_for_failed_close)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
  EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(-1));
  ASSERT_NO_THROW(nmpp::socket socket(domain, proto));
}

TEST_F(socket_test, socket_cannot_be_copied)
{
  ASSERT_FALSE(std::is_copy_constructible<nmpp::socket>::value);
  ASSERT_FALSE(std::is_copy_assignable<nmpp::socket>::value);
}

TEST_F(socket_test, socket_can_be_moved)
{
  ASSERT_TRUE(std::is_nothrow_move_constructible<nmpp::socket>::value);
  ASSERT_TRUE(std::is_nothrow_move_assignable<nmpp::socket>::value);
}

TEST_F(socket_test, socket_does_not_close_after_being_moved)
{
  InSequence seq;
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).Times(1).WillOnce(Return(1));
  EXPECT_CALL(nanomsg, nn_close(1)).Times(1).WillOnce(Return(0));

  {
    nmpp::socket socket(domain, proto);
    nmpp::socket socket2 = std::move(socket);
  }
}

TEST_F(socket_test, socket_closes_before_being_replaced)
{
  InSequence seq;
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).Times(1).WillOnce(Return(1));
  nmpp::socket socket(domain, proto);

  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).Times(1).WillOnce(Return(2));
  nmpp::socket socket2(domain, proto);

  EXPECT_CALL(nanomsg, nn_close(1)).Times(1).WillOnce(Return(0));
  socket = std::move(socket2);

  EXPECT_CALL(nanomsg, nn_close(2)).Times(1).WillOnce(Return(0));
}

struct socket_operation_test : Test
{
  void SetUp()
  {
    EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
    EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(0));
    socket.reset(new nmpp::socket(domain, proto));
  }

  int domain = 0;
  int proto = 0;
  nanomsg_mock nanomsg;
  std::unique_ptr<nmpp::socket> socket;
};

TEST_F(socket_operation_test, binds_endpoint)
{
  std::string endpoint = "tcp://localhost:5000";
  EXPECT_CALL(nanomsg, nn_bind(1, endpoint.c_str()));
  socket->bind(endpoint);
}

TEST_F(socket_operation_test, throws_exception_when_bind_returns_minus_one)
{
  std::string endpoint = "tcp://localhost:5000";
  EXPECT_CALL(nanomsg, nn_bind(1, endpoint.c_str())).WillOnce(Return(-1));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(socket->bind(endpoint), nmpp::exception);
}

TEST_F(socket_operation_test, connects_to_endpoint)
{
  std::string endpoint = "tcp://localhost:5000";
  EXPECT_CALL(nanomsg, nn_connect(1, endpoint.c_str()));
  socket->connect(endpoint);
}

TEST_F(socket_operation_test, throws_exception_when_connect_returns_minus_one)
{
  std::string endpoint = "tcp://localhost:5000";
  EXPECT_CALL(nanomsg, nn_connect(1, endpoint.c_str())).WillOnce(Return(-1));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(socket->connect(endpoint), nmpp::exception);
}

struct socket_send_receive_test : Test
{
  void SetUp()
  {
    EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
    EXPECT_CALL(nanomsg, nn_close(1)).WillOnce(Return(0));
    socket.reset(new nmpp::socket(domain, proto));
  }

  int domain = 0;
  int proto = 0;
  nanomsg_mock nanomsg;
  std::unique_ptr<nmpp::socket> socket;
  message_mock msg;
  static constexpr size_t length = 5;
  char data[length] = {1, 2, 3, 4, 5};
};

TEST_F(socket_send_receive_test, sends_and_releases_message)
{
  {
    InSequence seq;
    EXPECT_CALL(msg, valid()).WillOnce(Return(true));
    EXPECT_CALL(msg, release()).WillOnce(Return(data));
  }

  EXPECT_CALL(nanomsg, nn_send(1, _, NN_MSG, 0));
  socket->send(msg);
}

TEST_F(socket_send_receive_test, throws_when_message_is_invalid)
{
  EXPECT_CALL(msg, valid()).WillOnce(Return(false));
  ASSERT_THROW(socket->send(msg), std::logic_error);
}

TEST_F(socket_send_receive_test, throws_when_send_return_minus_one)
{
  EXPECT_CALL(msg, valid()).WillOnce(Return(true));
  EXPECT_CALL(msg, release()).WillOnce(Return(data));
  EXPECT_CALL(nanomsg, nn_send(1, _, NN_MSG, 0)).WillOnce(Return(-1));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(socket->send(msg), nmpp::exception);
}

TEST_F(socket_send_receive_test, receives_message)
{
  EXPECT_CALL(nanomsg, nn_recv(1, _, NN_MSG, 0)).WillOnce(Return(5));
  auto message = socket->receive<message_mock>();
  ASSERT_THAT(message->m_length, Eq(5));
}

TEST_F(socket_send_receive_test, throws_when_recv_return_minus_one)
{
  EXPECT_CALL(nanomsg, nn_recv(1, _, NN_MSG, 0)).WillOnce(Return(-1));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(socket->receive<message_mock>(), nmpp::exception);
}

ACTION_P(SetArgVoidPointer, ptr)
{
  *reinterpret_cast<void**>(arg1) = ptr;
}

TEST_F(socket_send_receive_test, received_message_has_payload)
{
  EXPECT_CALL(nanomsg, nn_recv(1, _, NN_MSG, 0))
      .WillOnce(DoAll(SetArgVoidPointer(data), Return(5)));
  auto message = socket->receive<message_mock>();
  ASSERT_THAT(message->m_length, Eq(5));
  ASSERT_THAT(message->m_message, Eq(data));
}

ACTION_P(SetArgVoidPointee, p)
{
  *reinterpret_cast<int*>(arg3) = p;
}

struct async_socket_test : Test
{
  void SetUp()
  {
    EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
    EXPECT_CALL(nanomsg, nn_getsockopt(1, NN_SOL_SOCKET, NN_RCVFD, _, _))
        .WillOnce(DoAll(SetArgVoidPointee(100), Return(0)));
    EXPECT_CALL(nanomsg, nn_getsockopt(1, NN_SOL_SOCKET, NN_SNDFD, _, _))
        .WillOnce(DoAll(SetArgVoidPointee(101), Return(0)));
    asocket.reset(new async_socket(domain, proto, io_service));
  }

  void TearDown()
  {
    EXPECT_CALL(nanomsg, nn_close(1)).Times(1).WillOnce(Return(0));
  }

  boost::asio::io_service io_service;
  int domain = 0;
  int proto = 0;
  nanomsg_mock nanomsg;
  using async_socket = nmpp::async_socket_impl<async_dispatcher_mock>;
  std::unique_ptr<async_socket> asocket;
};

TEST_F(async_socket_test, returns_minut_one_when_getsockopt_does_not_change_fd)
{
  EXPECT_CALL(nanomsg, nn_socket(domain, proto)).WillOnce(Return(1));
  EXPECT_CALL(nanomsg, nn_getsockopt(1, NN_SOL_SOCKET, NN_RCVFD, _, _))
      .WillOnce(Return(0));
  EXPECT_CALL(nanomsg, nn_getsockopt(1, NN_SOL_SOCKET, NN_SNDFD, _, _))
      .WillOnce(Return(0));
  async_socket socket(domain, proto, io_service);
  ASSERT_THAT(socket.get_async_dispatcher().m_receive_sock, Eq(-1));
  ASSERT_THAT(socket.get_async_dispatcher().m_send_sock, Eq(-1));
  EXPECT_CALL(nanomsg, nn_close(1)).Times(1).WillOnce(Return(0));
}

TEST_F(async_socket_test,
       create_async_socket_specialized_with_native_socket_implementation)
{
  ASSERT_THAT(asocket->get_async_dispatcher().m_receive_sock, Eq(100));
  ASSERT_THAT(asocket->get_async_dispatcher().m_send_sock, Eq(101));
}

TEST_F(async_socket_test, async_send_stores_handler)
{
  async_dispatcher_mock::handler handler;
  const async_dispatcher_mock& nsm = asocket->get_async_dispatcher();
  EXPECT_CALL(nsm, on_send_event(_)).WillOnce(SaveArg<0>(&handler));
  auto msg{std::make_unique<message_mock>()};

  static constexpr size_t length = 5;
  char data[length] = {1, 2, 3, 4, 5};
  EXPECT_CALL(*msg, valid()).WillOnce(Return(true));
  EXPECT_CALL(*msg, release()).WillOnce(Return(data));
  asocket->async_send(std::move(msg), [](const std::error_code&, size_t) {});

  EXPECT_CALL(nanomsg, nn_send(1, _, NN_MSG, 0)).WillOnce(Return(1));
  handler(std::error_code());
}

struct MessageReceiverMock
{
  MOCK_METHOD1(handle, void(const message_mock&));
};

TEST_F(async_socket_test, async_receive_stores_handler)
{
  MessageReceiverMock receiver;
  async_dispatcher_mock::handler handler;
  const async_dispatcher_mock& nsm = asocket->get_async_dispatcher();
  EXPECT_CALL(nsm, on_receive_event(_)).WillOnce(SaveArg<0>(&handler));

  static constexpr size_t length = 5;
  char data[length] = {1, 2, 3, 4, 5};
  asocket->async_receive<message_mock>(std::bind(&MessageReceiverMock::handle,
                                                 std::ref(receiver),
                                                 std::placeholders::_1));

  EXPECT_CALL(nanomsg, nn_recv(1, _, NN_MSG, 0))
      .WillOnce(DoAll(SetArgVoidPointer(data), Return(5)));
  EXPECT_CALL(receiver, handle(_));
  handler(std::error_code());
}
