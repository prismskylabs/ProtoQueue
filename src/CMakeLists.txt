set(PROTOQUEUE_LIBRARIES protoqueue CACHE INTERNAL "Main object library for protoqueue")
set(PROTOQUEUE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${ZEROMQ_INCLUDE_DIRS} CACHE INTERNAL
    "Main include directory for protoqueue")

add_library(${PROTOQUEUE_LIBRARIES}
    address.cpp
    port.cpp
    topic.cpp
    type.cpp)

target_include_directories(${PROTOQUEUE_LIBRARIES} PRIVATE
    ${PROTOQUEUE_INCLUDE_DIRS})

target_link_libraries(${PROTOQUEUE_LIBRARIES} PRIVATE
    ${ZEROMQ_LIBRARIES})
