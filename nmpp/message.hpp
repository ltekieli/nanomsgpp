#ifndef NMPP_MESSAGE_HPP_
#define NMPP_MESSAGE_HPP_

#include <nanomsg/nn.h>
#include <nmpp/exception.hpp>

namespace nmpp
{

class message
{
public:
  message(const message&) = delete;
  message& operator=(const message&) = delete;

  message(size_t length, char* payload = nullptr) noexcept : m_length(length),
                                                             m_message(payload)
  {
  }

  message(message&& rhs) noexcept : message(0)
  {
    *this = std::move(rhs);
  }

  message& operator=(message&& rhs) noexcept
  {
    cleanup();
    this->m_length = rhs.m_length;
    this->m_message = rhs.m_message;
    rhs.m_length = 0;
    rhs.m_message = nullptr;
  }

  ~message() noexcept
  {
    cleanup();
  }

  const char* data() const
  {
    return m_message;
  }

  void fill(const char* payload)
  {
    throw_when<std::logic_error>(valid(), "Message already contains data");
    m_message = reinterpret_cast<char*>(nn_allocmsg(m_length, 0));
    throw_when(m_message == nullptr);
    std::copy(payload, payload + m_length, m_message);
  }

  bool valid() const
  {
    return m_message != nullptr;
  }

  char* release()
  {
    auto temp = m_message;
    m_message = nullptr;
    return temp;
  }

  size_t size() const
  {
    if (valid())
    {
      return m_length;
    }
    return 0;
  }

private:
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
