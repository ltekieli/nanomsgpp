#ifndef NMPP_MESSAGE_HPP_
#define NMPP_MESSAGE_HPP_

#include <memory>
#include <nanomsg/nn.h>
#include <nmpp/exception.hpp>

namespace nmpp
{

class message
{
public:
  message(const message&) = delete;
  message& operator=(const message&) = delete;
  message(message&& rhs) = delete;
  message& operator=(message&& rhs) = delete;
  message() = delete;

  static auto from(const char* payload, size_t size)
  {
    auto msg = std::unique_ptr<message>(new message(size));
    msg->fill(payload);
    return std::move(msg);
  }

  static auto from_nn(void* nnmsg, size_t size)
  {
    return std::unique_ptr<message>(new message(size, nnmsg));
  }

  ~message() noexcept
  {
    cleanup();
  }

  const char* data() const
  {
    return m_message;
  }

  bool valid() const
  {
    return m_message != nullptr;
  }

  char* release()
  {
    auto temp = m_message;
    m_message = nullptr;
    m_length = 0;
    return temp;
  }

  size_t size() const
  {
    return m_length;
  }

private:
  message(size_t length) noexcept : m_length(length), m_message(nullptr)
  {
  }

  message(size_t length, void* payload) noexcept
      : m_length(length),
        m_message(reinterpret_cast<char*>(payload))
  {
  }

  void fill(const char* payload)
  {
    throw_when<std::logic_error>(valid(), "Message already contains data");
    m_message = reinterpret_cast<char*>(nn_allocmsg(m_length, 0));
    throw_when(m_message == nullptr);
    std::copy(payload, payload + m_length, m_message);
  }

  void cleanup()
  {
    if (valid())
      nn_freemsg(m_message);
  }

  size_t m_length;
  char* m_message;
};

} // namespace nmpp

#endif // NMPP_MESSAGE_HPP_
