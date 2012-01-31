install(FILES
        booredisasync.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/booredis COMPONENT Devel
)



set (booredis_SRCS booredisasync.cpp)

add_library (booredis SHARED ${booredis_SRCS})
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
