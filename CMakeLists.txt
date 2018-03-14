CMake_minimum_required(VERSION 3.0)

project(booredis VERSION 0.2.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

# compile in release with debug info mode by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

option(BUILD_EXAMPLES "Build booredis examples" ON)

# Subdirs
add_subdirectory(src)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


include (CTest)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


# CMake config to provide library detection on target system. New policy
# requires to expand target name at run-time and not at configure time, thus
# requiring to run file(GENERATE) after configure_file() in some cases.

# See https://cmake.org/pipermail/cmake/2015-April/060350.html for more
# details.

set(CMAKE_INSTALL_CMAKECONF ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/booredis-config.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKECONF})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
                                 COMPATIBILITY AnyNewerVersion)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/booredis-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/booredis-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_CMAKECONF}
        COMPONENT dev)

install(DIRECTORY include/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT dev)

# pkg-config
if(NOT WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/booredis.pc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/booredis.pc
                   @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/booredis.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

install(FILES COPYING README
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT doc)

# Install the export set for use with the install-tree
install(EXPORT booredis-targets
        FILE booredis-targets.cmake
        NAMESPACE Booredis::
        DESTINATION ${CMAKE_INSTALL_CMAKECONF}
        COMPONENT dev)

# Custom target to uninstall library and supplementary files
configure_file("${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE
               @ONLY)

add_custom_target(uninstall
                  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
