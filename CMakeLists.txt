project(nanomsg++)
cmake_minimum_required(VERSION 2.8)

message(STATUS "${CMAKE_MODULE_PATH}")

option(BUILD_TESTS "Include test targets" OFF)
message(STATUS "Include test targets: ${BUILD_TESTS}")

find_package(Boost 1.62 COMPONENTS system)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NANOMSG nanomsg)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${NANOMSG_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${NANOMSG_LIBRARY_DIRS})
add_definitions(${Boost_CFLAGS_OTHER})
add_definitions(${NANOMSG_CFLAGS_OTHER})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++14)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")

if(BUILD_TESTS)
  add_subdirectory(test/integration)
  add_subdirectory(test/ut)
endif(BUILD_TESTS)

install(
  DIRECTORY nmpp
  DESTINATION include
)
