IF(${BUILD_UNITTESTS})

enable_testing()
# Enable ExternalProject CMake module
INCLUDE(ExternalProject)

ExternalProject_Add(
        googletest
        PREFIX  ${CMAKE_BINARY_DIR}/googletest
        SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
        TIMEOUT 10
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
    )
SET(GTEST_DIR "${CMAKE_BINARY_DIR}/googletest/src/googletest")
SET(GTEST_INCLUDE_DIR "${GTEST_DIR}/include")
SET(GTEST_LIB_DIR "${CMAKE_BINARY_DIR}/googletest/src/googletest-build")

set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

add_executable(${PROJECT_TEST_NAME}
    ${TEST_SRC_FILES}
)
add_dependencies(${PROJECT_TEST_NAME} googletest)
include_directories(${GTEST_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(${PROJECT_TEST_NAME} ${PROJECT_LIB} ${COMMON_LIBRARIES} -L${GTEST_LIB_DIR} -lgtest -lgtest_main -lpthread)
add_dependencies(${PROJECT_TEST_NAME} ${GTEST_LIB_DIR}/libgtest.a ${GTEST_LIB_DIR}/libgtest_main.a)

ENDIF(${BUILD_UNITTESTS})
