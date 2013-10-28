CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(PROJECT_NAME_STR booredis)
project(${PROJECT_NAME_STR})


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_INSTALL_NAME_DIR ${LIB_INSTALL_DIR})

# compile in debug mode
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

#add extra search paths for libraries and includes
SET (LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
SET (LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Directory where lib will install")
SET (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")
SET (CMAKE_MODULES_INSTALL_DIR "${CMAKE_ROOT}/Modules" CACHE PATH "The directory to install Findbooredis.cmake to")


set(BOOREDIS_LIB_MAJOR_VERSION "0")
set(BOOREDIS_LIB_MINOR_VERSION "1")
set(BOOREDIS_LIB_PATCH_VERSION "2")

set(BOOREDIS_LIB_VERSION_STRING "${BOOREDIS_LIB_MAJOR_VERSION}.${BOOREDIS_LIB_MINOR_VERSION}.${BOOREDIS_LIB_PATCH_VERSION}")

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib${LIB_SUFFIX}" )

# pkg-config
IF (NOT WIN32)
  CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/booredis.pc.in
                  ${CMAKE_CURRENT_BINARY_DIR}/booredis.pc
                  @ONLY)
  INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/booredis.pc
           DESTINATION lib${LIB_SUFFIX}/pkgconfig)
ENDIF (NOT WIN32)

# Project dependencies
find_package(Boost COMPONENTS thread system REQUIRED)

set(
COMMON_INCLUDES
${PROJECT_SOURCE_DIR}/src
${Boost_INCLUE_DIR}
)

set(
COMMON_LIBRARIES
${Boost_LIBRARIES}
)
include_directories(${COMMON_INCLUDES})

SET(PROJECT_LIB booredis)

# Subdirs
add_subdirectory(cmake/modules)
add_subdirectory(src)


CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

GET_TARGET_PROPERTY(LIB_OUT_NAME ${PROJECT_LIB} LOCATION)
GET_FILENAME_COMPONENT(LIB_OUT_NAME  ${LIB_OUT_NAME} NAME)

# cmake-modules
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/booredis-config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
               @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/booredis-config-version.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
               @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
        DESTINATION "lib/cmake/booredis")

install(FILES
        COPYING
        README
        DESTINATION share/doc/libbooredis
)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Testing
SET(BUILD_UNITTESTS 1)
add_subdirectory(test)
