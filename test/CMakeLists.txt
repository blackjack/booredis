IF(${BUILD_UNITTESTS})

enable_testing()

INCLUDE(ExternalProject)
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/ThirdParty)

ExternalProject_Add(
        googletest
        PREFIX  ${CMAKE_BINARY_DIR}/googletest
        SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
        TIMEOUT 10
        INSTALL_COMMAND ""
        UPDATE_COMMAND "" #skip svn update
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
    )

ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)

ExternalProject_Get_Property(googletest binary_dir)
add_library(gtest UNKNOWN IMPORTED)
set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
add_dependencies(gtest googletest)

add_library(gtest_main UNKNOWN IMPORTED)
set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
add_dependencies(gtest_main googletest)

set(PROJECT_TEST_NAME test)

file(GLOB TEST_SRC_FILES
${PROJECT_SOURCE_DIR}/test/*.cpp
${PROJECT_SOURCE_DIR}/test/async/*.cpp
${PROJECT_SOURCE_DIR}/test/sync/*.cpp

${PROJECT_SOURCE_DIR}/test/*.h
${PROJECT_SOURCE_DIR}/test/async/*.h
${PROJECT_SOURCE_DIR}/test/sync/*.h
)

add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
target_link_libraries(${PROJECT_TEST_NAME} ${PROJECT_LIB} gtest gtest_main ${COMMON_LIBRARIES})
add_dependencies(${PROJECT_TEST_NAME} googletest)

ENDIF(${BUILD_UNITTESTS})
