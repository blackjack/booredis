install(FILES
        booredisasync.h
        booredissync.h
        redismessage.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/booredis COMPONENT Devel
)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

set (booredis_SRCS booredisasync.cpp booredissync.cpp redismessage.cpp)

add_library (booredis STATIC ${booredis_SRCS})
target_link_libraries(booredis  ${Boost_LIBRARIES})

set_target_properties(booredis PROPERTIES
                      VERSION ${BOOREDIS_LIB_MAJOR_VERSION}.${BOOREDIS_LIB_MINOR_VERSION}.${BOOREDIS_LIB_PATCH_VERSION}
                      SOVERSION ${BOOREDIS_LIB_MAJOR_VERSION}
                      DEFINE_SYMBOL BOOREDIS_MAKEDLL
                      )

INSTALL(TARGETS booredis
   LIBRARY DESTINATION ${LIB_INSTALL_DIR}
   RUNTIME DESTINATION bin
   ARCHIVE DESTINATION  ${LIB_INSTALL_DIR}
)

add_executable(async examples/async.cpp)
target_link_libraries(async booredis)

add_executable(sync examples/sync.cpp)
target_link_libraries(sync booredis)

install(FILES
        examples/async.cpp
        examples/sync.cpp
        DESTINATION share/doc/libbooredis/examples
)
