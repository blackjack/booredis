project(booredis)
cmake_minimum_required(VERSION 2.8)

find_package(Boost COMPONENTS thread system REQUIRED)

set(SOURCES
    src/redisasyncclient.cpp
)

include_directories(${Boost_INCLUDE_DIR})

add_library(booredis SHARED ${SOURCES})
add_executable(example src/examples/example.cpp)


target_link_libraries(booredis ${Boost_LIBRARIES})
target_link_libraries(example booredis)
