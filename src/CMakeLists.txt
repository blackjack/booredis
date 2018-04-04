set(booredis_SRCS
    async.cpp
    sync.cpp
    decoder.cpp
    redismessage.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.42 REQUIRED COMPONENTS thread system)

if (BOOREDIS_BUILD_SHARED)
  set(library_type SHARED)
else()
  set(library_type STATIC)
endif()

add_library(booredis
            ${library_type}
            ${booredis_SRCS})

target_include_directories(booredis
                           PUBLIC
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

target_include_directories(booredis SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(booredis
                      PUBLIC ${Boost_LIBRARIES}
                      PRIVATE ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(booredis
                      PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED YES)

set_target_properties(booredis
                      PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR}
                      DEFINE_SYMBOL "BOOREDIS_MAKEDLL")

install(TARGETS booredis
        EXPORT booredis-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR})
