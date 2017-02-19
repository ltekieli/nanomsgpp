#ifndef NMPP_SOCKET_HPP_
#define NMPP_SOCKET_HPP_

#include <nanomsg/bus.h>
#include <nanomsg/inproc.h>
#include <nanomsg/ipc.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/survey.h>
#include <nanomsg/tcp.h>
#include <nanomsg/ws.h>
#include <nmpp/exception.hpp>

#include <boost/asio.hpp>

namespace nmpp
{

class socket
{
public:
  socket(const socket&) = delete;
  socket& operator=(const socket&) = delete;

  socket(int domain, int proto) throw(exception) : m_sock(-1)
  {
    m_sock = nn_socket(domain, proto);
    throw_when(m_sock < 0);
  }

  socket(socket&& rhs) noexcept : m_sock(-1)
  {
    *this = std::move(rhs);
  }

  socket& operator=(socket&& rhs) noexcept
  {
    cleanup();
    this->m_sock = rhs.m_sock;
    rhs.m_sock = -1;
    return *this;
  }

  ~socket() noexcept
  {
    cleanup();
  }

  void close() throw(exception)
  {
    if (m_sock < 0)
      return;
    auto status = nn_close(m_sock);
    m_sock = -1;
    throw_when(status == -1);
  }

  void bind(const std::string& address) throw(exception)
  {
    throw_when(nn_bind(m_sock, address.c_str()) == -1);
  }

  void connect(const std::string& address) throw(exception)
  {
    throw_when(nn_connect(m_sock, address.c_str()) == -1);
  }

  template <typename message_type>
  void send(message_type&& msg) throw(std::logic_error, exception)
  {
    throw_when<std::logic_error>(!msg.valid(), "Invalid message");
    send(msg.release());
  }

  template <typename message_type> message_type receive() throw(exception)
  {
    char* buf = nullptr;
    size_t bytes_received = nn_recv(m_sock, &buf, NN_MSG, 0);
    throw_when(bytes_received == -1);
    return {bytes_received, buf};
  }

protected:
  size_t send(char* buf) throw(exception)
  {
    auto bytes_transferred = nn_send(m_sock, &buf, NN_MSG, 0);
    throw_when(bytes_transferred == -1);
    return bytes_transferred;
  }

  int get_receive_descriptor()
  {
    return get_native_descriptor(NN_RCVFD);
  }

  int get_send_descriptor()
  {
    return get_native_descriptor(NN_SNDFD);
  }

  int get_native_descriptor(int direction)
  {
    int sock = -1;
    size_t sock_size = sizeof(sock);
    nn_getsockopt(m_sock, NN_SOL_SOCKET, direction, &sock, &sock_size);
    return sock;
  }

  void cleanup() noexcept
  {
    try
    {
      this->close();
    }
    catch (...)
    {
    }
  }

  int m_sock;
};

template <typename async_dispatcher_type>
class async_socket_impl : public socket
{
public:
  template <typename... Args>
  async_socket_impl(int domain, int proto, Args&&... args) throw(exception)
      : socket(domain, proto),
        async_dispatcher(get_receive_descriptor(), get_send_descriptor(),
                         std::forward<Args>(args)...)
  {
  }

  const async_dispatcher_type& get_async_dispatcher()
  {
    return async_dispatcher;
  }

  template <typename message_type, typename handler_type>
  void async_send(message_type&& msg, handler_type&& handler)
  {
    throw_when<std::logic_error>(!msg.valid(), "Invalid message");
    auto msg_ptr =
        std::make_shared<typename std::remove_reference<message_type>::type>(
            msg.size(), msg.release());

    async_dispatcher.on_send_event(
        [this, handler, msg_ptr](const std::error_code& ec) {
          auto buf = msg_ptr->release();
          auto bytes = send(buf);
          handler(ec, bytes);
        });
  }

  template <typename message_type, typename handler_type>
  void async_receive(handler_type&& handler)
  {
    async_dispatcher.on_receive_event(
        [this, handler](const std::error_code& ec) {
          auto&& msg = receive<message_type>();
          handler(msg);
        });
  }

private:
  async_dispatcher_type async_dispatcher;
};

} // namespace nmpp

#include <nmpp/async_dispatcher.hpp>
#include <nmpp/native_socket.hpp>

namespace nmpp
{
using async_socket = async_socket_impl<async_dispatcher<native_socket>>;
} // namespace nmpp

#endif // NMPP_SOCKET_HPP_
