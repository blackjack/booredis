find_package(GTestSrc REQUIRED)
enable_testing()

add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

set(TEST_LIBRARIES gtest gtest_main)

set(UNIT_TEST_TARGETS "")
macro(add_unit_test target)
    set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${target} PARENT_SCOPE)
    add_test(${target} ${CMAKE_CURRENT_BINARY_DIR}/${target})
endmacro(add_unit_test)

add_subdirectory(async)
add_subdirectory(sync)

add_custom_target(tests ALL DEPENDS ${UNIT_TEST_TARGETS})
