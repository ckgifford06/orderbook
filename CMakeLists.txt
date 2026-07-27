cmake_minimum_required(VERSION 3.15)
project(orderbook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wextra")

add_executable(orderbook
    src/main.cpp
    src/order_book.cpp
)

target_include_directories(orderbook PRIVATE include)
