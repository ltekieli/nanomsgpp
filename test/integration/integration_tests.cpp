#include <functional>
#include <gmock/gmock.h>
#include <thread>

#include <nmpp/async_dispatcher.hpp>
#include <nmpp/message.hpp>
#include <nmpp/native_socket.hpp>
#include <nmpp/socket.hpp>

using namespace ::testing;

std::string random_string(size_t length)
{
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

void send_handler(nmpp::async_socket& socket, const std::error_code ec,
                  size_t bytes)
{
  auto data = random_string(31);
  auto msg = nmpp::message::from(data.c_str(), data.size() + 1);
  socket.async_send(std::move(msg),
                    [&socket](const std::error_code ec, size_t bytes) {
                      send_handler(socket, ec, bytes);
                    });
}

void receive_handler(nmpp::async_socket& socket, const nmpp::message& msg)
{
  socket.async_receive<nmpp::message>(
      [&socket](const nmpp::message& msg) { receive_handler(socket, msg); });
}

TEST(integration_test, create_socket)
{
  boost::asio::io_service io;

  nmpp::async_socket pull_socket(AF_SP, NN_PULL, io);
  pull_socket.bind("tcp://127.0.0.1:5555");

  nmpp::async_socket push_socket(AF_SP, NN_PUSH, io);
  push_socket.connect("tcp://127.0.0.1:5555");

  auto data = random_string(31);
  auto msg = nmpp::message::from(data.c_str(), data.size() + 1);
  push_socket.async_send(
      std::move(msg), [&push_socket](const std::error_code ec, size_t bytes) {
        send_handler(push_socket, ec, bytes);
      });

  pull_socket.async_receive<nmpp::message>([&pull_socket](
      const nmpp::message& msg) { receive_handler(pull_socket, msg); });

  std::thread t1([&io]() { io.run(); });
  std::this_thread::sleep_for(std::chrono::seconds(1));
  io.stop();
  t1.join();
}
