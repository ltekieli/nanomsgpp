#include "mocks/nanomsg_mock.hpp"
#include <gtest/gtest.h>
#include <nmpp/message.hpp>
#include <type_traits>

using namespace ::testing;

struct message_test : Test
{
  void SetUp()
  {
    EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
        .WillOnce(Return(allocatedMemory));
    message = std::move(nmpp::message::from(payload, length));
  }

  void TearDown()
  {
    EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
    message.reset();
  }

  static constexpr size_t length = 5;
  char allocatedMemory[length] = {0, 0, 0, 0, 0};
  char allocatedMemory2[length] = {0, 0, 0, 0, 0};
  char payload[length] = {1, 2, 3, 4, 5};

  nanomsg_mock nanomsg;
  std::unique_ptr<nmpp::message> message;
};

TEST_F(message_test, payload_is_copied_to_allocated_memory)
{
  ASSERT_THAT(std::vector<char>(message->data(), message->data() + length),
              ElementsAreArray(payload));
}

TEST_F(message_test, throws_on_bad_allocation)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0)).WillOnce(Return(nullptr));
  EXPECT_CALL(nanomsg, nn_errno()).WillOnce(Return(1));
  ASSERT_THROW(nmpp::message::from(payload, length), nmpp::exception);
}

TEST(message_copy_test, message_cannot_be_copied)
{
  ASSERT_FALSE(std::is_copy_constructible<nmpp::message>::value);
  ASSERT_FALSE(std::is_copy_assignable<nmpp::message>::value);
}

TEST(message_move_test, message_cannot_be_moved)
{
  ASSERT_FALSE(std::is_nothrow_move_constructible<nmpp::message>::value);
  ASSERT_FALSE(std::is_nothrow_move_assignable<nmpp::message>::value);
}

TEST_F(message_test, message_frees_memory_on_destruction)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory2));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory2));
  {
    nmpp::message::from(payload, length);
  }
}

TEST_F(message_test, message_releases_memory_on_request)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  auto message = nmpp::message::from(payload, length);
  auto raw_payload = message->release();
  ASSERT_THAT(raw_payload, Eq(allocatedMemory));
  ASSERT_THAT(message->valid(), Eq(false));
}

TEST_F(message_test, factory_method_for_constructing_outgoing_messages)
{
  EXPECT_CALL(nanomsg, nn_allocmsg(length, 0))
      .WillOnce(Return(allocatedMemory));
  auto message = nmpp::message::from(payload, length);
  ASSERT_THAT(message->size(), Eq(length));
  ASSERT_THAT(message->data(), Eq(allocatedMemory));
  EXPECT_CALL(nanomsg, nn_freemsg(allocatedMemory));
}

TEST_F(message_test, factory_method_for_constructing_incoming_messages)
{
  auto message = nmpp::message::from_nn(payload, length);
  ASSERT_THAT(message->size(), Eq(length));
  ASSERT_THAT(message->data(), Eq(payload));
  EXPECT_CALL(nanomsg, nn_freemsg(payload));
}
