cmake_minimum_required(VERSION 3.17)
project(twitter_sisop2)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(twitter_sisop2 main.cpp twitter_server.cpp twitter_server.h twitter_client.cpp twitter_client.h)
