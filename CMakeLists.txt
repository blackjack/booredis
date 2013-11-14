CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

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
SET (LIB_SUFFIX CACHE STRING "Define suffix of directory name (32/64)" )
SET (LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE STRING "Directory where lib will install")
SET (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The directory the headers are installed in")

#We should not really install Findbooredis.cmake anywhere
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Findbooredis.cmake.in
#               ${CMAKE_CURRENT_BINARY_DIR}/Findbooredis.cmake
#               @ONLY)
#INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/Findbooredis.cmake
#         DESTINATION "${CMAKE_ROOT}/Modules")


set(LIB_MAJOR_VERSION "0")
set(LIB_MINOR_VERSION "1")
set(LIB_PATCH_VERSION "2")

set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib/${CMAKE_LIBRARY_ARCHITECTURE}" )

# pkg-config
IF (NOT WIN32)
  CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME_STR}.pc.in
                  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_STR}.pc
                  @ONLY)
  INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_STR}.pc
           DESTINATION "lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")
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

SET(PROJECT_LIB ${PROJECT_NAME_STR})

add_subdirectory(src)

CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

GET_TARGET_PROPERTY(LIB_OUT_NAME ${PROJECT_LIB} LOCATION)
GET_FILENAME_COMPONENT(LIB_OUT_NAME ${LIB_OUT_NAME} NAME)

# cmake-modules
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/booredis-config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
               @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/booredis-config-version.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
               @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
        DESTINATION "lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/${PROJECT_NAME_STR}")

install(FILES
        COPYING
        README
        DESTINATION "share/doc/lib${PROJECT_NAME_STR}"
)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Testing
SET (BUILD_UNITTESTS Yes CACHE STRING "Automatically build unit tests")
add_subdirectory(test)
