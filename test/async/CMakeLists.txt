add_executable(booredis-test-async 
    bulk.cpp
    error.cpp
    integer.cpp
    multibulk.cpp
    string.cpp)

target_link_libraries(booredis-test-async booredis ${TEST_LIBRARIES})
add_unit_test(booredis-test-async)
