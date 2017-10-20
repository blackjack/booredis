CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

project(booredis)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

include(GNUInstallDirs)

set(LIB_MAJOR_VERSION "0")
set(LIB_MINOR_VERSION "1")
set(LIB_PATCH_VERSION "6")

set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")


# compile in release with debug info mode by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)


OPTION(BOOREDIS_BUILD_SHARED "Build booredis as a shared library" ON)
OPTION(BOOREDIS_BUILD_CXX11 "Build booredis with C++11" ON)
OPTION(BOOREDIS_BUILD_EXAMPLES "Build booredis examples" ON)
OPTION(BOOREDIS_BUILD_UNITTESTS "Automatically build unit tests" ON)


# Select to build shared or static library
IF(BOOREDIS_BUILD_SHARED)
    SET(BOOREDIS_LIBRARY_TYPE SHARED)
else()
    SET(BOOREDIS_LIBRARY_TYPE STATIC)
ENDIF()

IF(BOOREDIS_BUILD_CXX11)
    ADD_DEFINITIONS("-std=c++11")
ENDIF()

include_directories(include)
# Subdirs
add_subdirectory(src)
add_subdirectory(examples)

if(BOOREDIS_BUILD_UNITTESTS)
    add_subdirectory(test)
endif()


EXPORT( PACKAGE ${PROJECT_NAME} )

# CMake config to provide library detection on target system. New policy
# requires to expand target name at run-time and not at configure time, thus
# requiring to run file(GENERATE) after configure_file() in some cases.

# See https://cmake.org/pipermail/cmake/2015-April/060350.html for more
# details.

SET(CMAKE_INSTALL_CMAKECONF ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.pre
               @ONLY)
FILE(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INPUT  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.pre)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config-version.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
               @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
              DESTINATION ${CMAKE_INSTALL_CMAKECONF}
        COMPONENT dev)

install(DIRECTORY include/${PROJECT_NAME} 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev)

# pkg-config
IF (NOT WIN32)
    CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        @ONLY)
    INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
ENDIF (NOT WIN32)

install(FILES
    COPYING
    README
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    COMPONENT doc)

# Install the export set for use with the install-tree
install(EXPORT booredis-targets DESTINATION "${CMAKE_INSTALL_CMAKECONF}" COMPONENT dev)

# Custom target to uninstall library and supplementary files
CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
