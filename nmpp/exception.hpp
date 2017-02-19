#ifndef NMPP_EXCEPTION_HPP_
#define NMPP_EXCEPTION_HPP_

#include <exception>
#include <nanomsg/nn.h>

namespace nmpp
{

class exception final : std::exception
{
public:
  exception() : m_err(nn_errno())
  {
  }

  int num() const noexcept
  {
    return m_err;
  }

  virtual const char* what() const throw()
  {
    return nn_strerror(m_err);
  }

private:
  int m_err;
};

inline void throw_when(bool condition)
{
  if (condition)
    throw exception();
}

template <typename exception_type, typename... Args>
inline void throw_when(bool condition, Args&&... args)
{
  if (condition)
    throw exception_type(std::forward<Args>(args)...);
}

} // namespace nmpp

#endif // NMPP_EXCEPTION_HPP_
