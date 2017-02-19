#ifndef NANOMSG_MOCK_HPP_
#define NANOMSG_MOCK_HPP_

#include <errno.h>
#include <gmock/gmock.h>

struct nanomsg_mock
{
  nanomsg_mock();
  MOCK_METHOD2(nn_socket, int(int domain, int protocol));
  MOCK_METHOD0(nn_errno, int());
  MOCK_METHOD1(nn_strerror, const char*(int));
  MOCK_METHOD1(nn_close, int(int));
  MOCK_METHOD2(nn_bind, int(int, const char*));
  MOCK_METHOD2(nn_connect, int(int, const char*));
  MOCK_METHOD2(nn_allocmsg, void*(size_t, int));
  MOCK_METHOD1(nn_freemsg, int(void*));
  MOCK_METHOD4(nn_send, int(int, const void*, size_t, int));
  MOCK_METHOD5(nn_getsockopt, int(int, int, int, void*, size_t*));
  MOCK_METHOD4(nn_recv, int(int, void*, size_t, int));
};

#endif // NANOMSG_MOCK_HPP_
