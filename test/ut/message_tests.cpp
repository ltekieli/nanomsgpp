#include "mocks/nanomsg_mock.hpp"
#include <gtest/gtest.h>
#include <nmpp/message.hpp>
#include <type_traits>

using namespace ::testing;

struct message_test : Test
{
  static constexpr size_t length = 5;
  char allocatedMemory[length] = {0, 0, 0, 0, 0};
  char allocatedMemory2[length] = {0, 0, 0, 0, 0};
  char payload[length] = {1, 2, 3, 4, 5};

  nanomsg_mock nanomsg;
  nmpp::message message = std::move(nmpp::message(length));
};

TEST_F(message_test, construct_base_on_length_data_is_null)
{
  size_t length = 10;
  nmpp::message message(length);
  ASSERT_THAT(message.data(), Eq(nullptr));
}

TEST_F(message_test, fills_allocated_message_with_given_data)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  message.fill(payload);
  ASSERT_THAT(allocatedMemory, ElementsAreArray(payload));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST_F(message_test, throws_on_bad_allocation)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0)).WillOnce(Return(nullptr));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(message.fill(payload), nmpp::exception);
}

TEST_F(message_test, data_is_equal_to_given_payload_after_fill)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  message.fill(payload);
  ASSERT_THAT(std::vector<char>(message.data(), message.data() + length),
              ElementsAreArray(payload));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST_F(message_test, message_is_invalid_when_was_not_filled)
{
  ASSERT_THAT(message.valid(), Eq(false));
}

TEST_F(message_test, message_is_valid_when_was_filled)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  message.fill(payload);
  ASSERT_THAT(message.valid(), Eq(true));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST(message_copy_test, message_cannot_be_copied)
{
  ASSERT_FALSE(std::is_copy_constructible<nmpp::message>::value);
  ASSERT_FALSE(std::is_copy_assignable<nmpp::message>::value);
}

TEST(message_move_test, message_can_be_moved)
{
  ASSERT_TRUE(std::is_nothrow_move_constructible<nmpp::message>::value);
  ASSERT_TRUE(std::is_nothrow_move_assignable<nmpp::message>::value);
}

TEST_F(message_test, message_is_not_valid_after_move)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  message.fill(payload);
  nmpp::message message2 = std::move(message);
  ASSERT_THAT(message.valid(), Eq(false));
  ASSERT_THAT(message2.valid(), Eq(true));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST_F(message_test, message_frees_memory_on_destruction)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory2));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory2));
  {
    nmpp::message message2(length);
    message2.fill(payload);
  }
}

TEST_F(message_test, message_frees_memory_on_move_after_fill)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory2));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory2));
  {
    message.fill(payload);
    message = std::move(nmpp::message(0));
  }
}

TEST_F(message_test, message_releases_memory_on_request)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory)).Times(0);
  message.fill(payload);
  auto raw_payload = message.release();
  ASSERT_THAT(raw_payload, Eq(allocatedMemory));
}

TEST_F(message_test, constructs_from_raw_payload_and_length)
{
  nmpp::message message(length, allocatedMemory);
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST_F(message_test, throws_error_when_filling_valid_message)
{
  nmpp::message message(length, allocatedMemory);
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
  ASSERT_THROW(message.fill(allocatedMemory2), std::logic_error);
}

TEST_F(message_test, returns_message_length_zero_when_invalid)
{
  nmpp::message message(length);
  ASSERT_THAT(message.size(), Eq(0));
}

TEST_F(message_test, returns_message_length_zero_when_valid)
{
  nmpp::message message(length, allocatedMemory);
  ASSERT_THAT(message.size(), Eq(length));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}
