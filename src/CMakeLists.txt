project(RealTimeDataTransfer)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

SET (Libraries Timers Senders Systems Utils -pthread)

add_subdirectory(senders)
add_subdirectory(systems)
add_subdirectory(timers)
add_subdirectory(utils)
add_subdirectory(tests)