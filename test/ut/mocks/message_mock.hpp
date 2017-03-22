#ifndef MESSAGE_MOCK_HPP_
#define MESSAGE_MOCK_HPP_

#include <errno.h>
#include <gmock/gmock.h>

struct message_mock
{
  message_mock(const message_mock&) = delete;
  message_mock& operator=(const message_mock&) = delete;

  message_mock(size_t length = 0, char* payload = nullptr) noexcept
      : m_length(length),
        m_message(payload)
  {
  }

  static auto from(char* payload, size_t size)
  {
    return std::make_unique<message_mock>(size, payload);
  }

  static auto from_nn(void* payload, size_t size)
  {
    return std::make_unique<message_mock>(size,
                                          reinterpret_cast<char*>(payload));
  }

  MOCK_CONST_METHOD0(data, const char*());
  MOCK_METHOD1(fill, void(const char*));
  MOCK_CONST_METHOD0(valid, bool());
  MOCK_METHOD0(release, char*());
  MOCK_METHOD0(size, size_t());

  size_t m_length;
  char* m_message;
};

#endif // MESSAGE_MOCK_HPP_
