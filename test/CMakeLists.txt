PROTOBUF_GENERATE_CPP(BASIC_PROTO_SRCS BASIC_PROTO_HDRS basic.proto)

add_executable(protoqueue_tests
    protoqueue_tests.cpp
    ${BASIC_PROTO_SRCS} ${BASIC_PROTO_HDRS})

target_include_directories(protoqueue_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOQUEUE_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

target_link_libraries(protoqueue_tests
    ${GTEST_BOTH_LIBRARIES}
    ${PROTOQUEUE_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME protoqueue_tests COMMAND protoqueue_tests)
