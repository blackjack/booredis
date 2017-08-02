if(BOOREDIS_BUILD_EXAMPLES)
    include_directories("../src")

    add_executable(async async.cpp)
    target_link_libraries(async ${PROJECT_NAME})

    add_executable(sync sync.cpp)
    target_link_libraries(sync ${PROJECT_NAME})
endif()

install(FILES async.cpp sync.cpp
    DESTINATION ${DOC_INSTALL_DIR}/examples)
