enable_testing()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/module")

find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})

set(TEST_EXECUTABLE_NAME ${PROJECT_NAME}-ut)
add_executable(${TEST_EXECUTABLE_NAME}
    # production code files
    ${PROJECT_SOURCE_DIR}/nmpp/async_dispatcher.hpp
    ${PROJECT_SOURCE_DIR}/nmpp/exception.hpp
    ${PROJECT_SOURCE_DIR}/nmpp/message.hpp
    ${PROJECT_SOURCE_DIR}/nmpp/native_socket.hpp
    ${PROJECT_SOURCE_DIR}/nmpp/socket.hpp
    main.cpp

    # mocks
    mocks/async_dispatcher_mock.hpp
    mocks/message_mock.hpp
    mocks/nanomsg_mock.cpp
    mocks/nanomsg_mock.hpp
    mocks/native_socket_mock.hpp

    # tests
    async_dispatcher_tests.cpp
    exception_tests.cpp
    message_tests.cpp
    socket_tests.cpp
)

target_link_libraries(${TEST_EXECUTABLE_NAME}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${Boost_LIBRARIES}
    pthread
)

add_test(nanomsg++-tests ${TEST_EXECUTABLE_NAME})
