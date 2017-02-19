#include <cassert>
#include <functional>
#include <nanomsg/nn.h>

#include "nanomsg_mock.hpp"

namespace
{
std::function<int(int, int)> nn_socket_cb;
std::function<int()> nn_errno_cb;
std::function<const char*(int)> nn_strerror_cb;
std::function<int(int)> nn_close_cb;
std::function<int(int, const char*)> nn_bind_cb;
std::function<int(int, const char*)> nn_connect_cb;
std::function<void*(size_t, int)> nn_allocmsg_cb;
std::function<int(void*)> nn_freemsg_cb;
std::function<int(int, const void*, size_t, int)> nn_send_cb;
std::function<int(int, int, int, void*, size_t*)> nn_getsockopt_cb;
std::function<int(int, void*, size_t, int)> nn_recv_cb;
}

nanomsg_mock::nanomsg_mock()
{
  nn_socket_cb = std::bind(&nanomsg_mock::nn_socket, this,
                           std::placeholders::_1, std::placeholders::_2);
  nn_errno_cb = std::bind(&nanomsg_mock::nn_errno, this);
  nn_strerror_cb =
      std::bind(&nanomsg_mock::nn_strerror, this, std::placeholders::_1);
  nn_close_cb =
      std::bind(&nanomsg_mock::nn_close, this, std::placeholders::_1);
  nn_bind_cb = std::bind(&nanomsg_mock::nn_bind, this, std::placeholders::_1,
                         std::placeholders::_2);
  nn_connect_cb = std::bind(&nanomsg_mock::nn_connect, this,
                            std::placeholders::_1, std::placeholders::_2);
  nn_allocmsg_cb = std::bind(&nanomsg_mock::nn_allocmsg, this,
                             std::placeholders::_1, std::placeholders::_2);
  nn_freemsg_cb =
      std::bind(&nanomsg_mock::nn_freemsg, this, std::placeholders::_1);
  nn_send_cb = std::bind(&nanomsg_mock::nn_send, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3,
                         std::placeholders::_4);
  nn_getsockopt_cb =
      std::bind(&nanomsg_mock::nn_getsockopt, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3,
                std::placeholders::_4, std::placeholders::_5);
  nn_recv_cb = std::bind(&nanomsg_mock::nn_recv, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3,
                         std::placeholders::_4);
}

int nn_socket(int domain, int protocol)
{
  assert(nn_socket_cb);
  return nn_socket_cb(domain, protocol);
}

int nn_errno()
{
  assert(nn_errno_cb);
  return nn_errno_cb();
}

const char* nn_strerror(int errnum)
{
  assert(nn_strerror_cb);
  return nn_strerror_cb(errnum);
}

int nn_close(int sock)
{
  assert(nn_close_cb);
  return nn_close_cb(sock);
}

int nn_bind(int sock, const char* addr)
{
  assert(nn_bind_cb);
  return nn_bind_cb(sock, addr);
}

int nn_connect(int sock, const char* addr)
{
  assert(nn_connect_cb);
  return nn_connect_cb(sock, addr);
}

void* nn_allocmsg(size_t size, int type)
{
  assert(nn_allocmsg_cb);
  return nn_allocmsg_cb(size, type);
}

int nn_freemsg(void* msg)
{
  assert(nn_freemsg_cb);
  return nn_freemsg_cb(msg);
}

int nn_send(int sock, const void* buf, size_t len, int flags)
{
  assert(nn_send_cb);
  return nn_send_cb(sock, buf, len, flags);
}

int nn_getsockopt(int s, int level, int option, void* optval,
                  size_t* optvallen)
{
  assert(nn_getsockopt_cb);
  return nn_getsockopt_cb(s, level, option, optval, optvallen);
}

int nn_recv(int s, void* buf, size_t len, int flags)
{
  assert(nn_recv_cb);
  return nn_recv_cb(s, buf, len, flags);
}
