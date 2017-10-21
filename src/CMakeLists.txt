set (booredis_SRCS 
    async.cpp 
    sync.cpp 
    decoder.cpp
    redismessage.cpp) 

find_package(Boost COMPONENTS thread system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(${PROJECT_NAME} ${BOOREDIS_LIBRARY_TYPE} ${booredis_SRCS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION "${LIB_VERSION_STRING}"
    SOVERSION "${LIB_MAJOR_VERSION}"
    DEFINE_SYMBOL "BOOREDIS_MAKEDLL")

INSTALL(TARGETS booredis
   EXPORT booredis-targets
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION bin
   ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
)
