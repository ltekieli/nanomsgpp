.ONESHELL:
SHELL=/bin/bash

PROCESSORS = $(shell nproc)

BUILD_DIR	:= ./build
RM			:= rm -rf

EXECUTABLE_NAME := "nanomsg++"
TEST_EXECUTABLE_NAME := ${EXECUTABLE_NAME}-ut
INT_TEST_EXECUTABLE_NAME := ${EXECUTABLE_NAME}-integration

.PHONY: all clean

all: app ut int

deps:
	set -e
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cd $(BUILD_DIR)

ut: deps
	set -e
	cd $(BUILD_DIR)
	cmake ../ -DBUILD_TESTS=ON -DBOOST_ROOT=/opt/boost_1_63_0
	make -j ${PROCESSORS} ${TEST_EXECUTABLE_NAME}
	./test/ut/${TEST_EXECUTABLE_NAME}

int: deps
	set -e
	cd $(BUILD_DIR)
	cmake ../ -DBUILD_TESTS=ON -DBOOST_ROOT=/opt/boost_1_63_0
	make -j ${PROCESSORS} ${INT_TEST_EXECUTABLE_NAME}
	./test/integration/${INT_TEST_EXECUTABLE_NAME}

app: deps
	set -e
	cd $(BUILD_DIR)
	cmake ../ -DBUILD_TESTS=OFF -DBOOST_ROOT=/opt/boost_1_63_0
	make -j ${PROCESSORS} all

install: app
	set -e
	cd $(BUILD_DIR)
	cmake ../ -DBUILD_TESTS=OFF -DBOOST_ROOT=/opt/boost_1_63_0
	make install

format:
	set -e
	find nmpp -name *.hpp -or -name *.h | xargs clang-format -i
	find test -name *.hpp -or -name *.h -or -name *.cpp | xargs clang-format -i

clean:
	$(RM) $(BUILD_DIR)
