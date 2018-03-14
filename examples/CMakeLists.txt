add_executable(async async.cpp)
target_link_libraries(async booredis)

add_executable(sync sync.cpp)
target_link_libraries(sync booredis)

install(FILES async.cpp sync.cpp
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
        COMPONENT doc)
