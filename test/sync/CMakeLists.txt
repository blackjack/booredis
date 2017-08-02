add_executable(${PROJECT_NAME}-test-sync 
    bulk.cpp
    error.cpp
    integer.cpp
    multibulk.cpp
    string.cpp)

target_link_libraries(${PROJECT_NAME}-test-sync ${PROJECT_NAME} ${TEST_LIBRARIES} ${Boost_LIBRARIES})
add_unit_test(${PROJECT_NAME}-test-sync)
