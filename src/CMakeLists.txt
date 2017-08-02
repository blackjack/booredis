set (booredis_SRCS 
    booredisasync.cpp 
    booredissync.cpp 
    redismessage.cpp 
    booredisdecoder.cpp)

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
   LIBRARY DESTINATION ${LIB_INSTALL_DIR}
   RUNTIME DESTINATION bin
   ARCHIVE DESTINATION  ${LIB_INSTALL_DIR}
)

install(FILES
    booredisasync.h
    booredissync.h
    redismessage.h
    booredisdecoder.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT dev
)
