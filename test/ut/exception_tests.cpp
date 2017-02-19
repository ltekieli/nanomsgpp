#include "mocks/nanomsg_mock.hpp"
#include <gtest/gtest.h>
#include <nmpp/socket.hpp>

using namespace ::testing;

TEST(exception_test, conveys_proper_error_code)
{
  nanomsg_mock nanomsg;
  try
  {
    EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
    throw nmpp::exception();
  }
  catch (const nmpp::exception& e)
  {
    ASSERT_THAT(e.num(), Eq(1));
  }
}

TEST(exception_test, conveys_proper_error_message)
{
  nanomsg_mock nanomsg;
  try
  {
    EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
    throw nmpp::exception();
  }
  catch (const nmpp::exception& e)
  {
    EXPECT_CALL(nanomsg, nn_strerror(1))
        .WillOnce(Return("Some error message"));
    ASSERT_THAT(e.what(), Eq("Some error message"));
  }
}

TEST(exception_test, throw_when_condition_is_fullfilled)
{
  nanomsg_mock nanomsg;
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(nmpp::throw_when(true), nmpp::exception);
}
