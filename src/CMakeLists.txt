install(FILES
        booredisasync.h
        booredissync.h
        redismessage.h
        booredisdecoder.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/booredis COMPONENT Devel
)

#add C++11 compile flag to be able to work with programs using it
add_definitions("-std=c++11")

set (booredis_SRCS booredisasync.cpp booredissync.cpp redismessage.cpp booredisdecoder.cpp)

add_library (${PROJECT_LIB} STATIC ${booredis_SRCS})
target_link_libraries(booredis  ${Boost_LIBRARIES})

set_target_properties(${PROJECT_LIB} PROPERTIES
                      VERSION ${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}
                      SOVERSION ${LIB_MAJOR_VERSION}
                      DEFINE_SYMBOL BOOREDIS_MAKEDLL
                      )

INSTALL(TARGETS booredis
   EXPORT booredis-targets
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
