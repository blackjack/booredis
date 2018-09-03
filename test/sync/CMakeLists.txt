add_executable(booredis-test-sync 
    bulk.cpp
    error.cpp
    integer.cpp
    multibulk.cpp
    string.cpp)

target_link_libraries(booredis-test-sync booredis ${TEST_LIBRARIES})
add_unit_test(booredis-test-sync)
