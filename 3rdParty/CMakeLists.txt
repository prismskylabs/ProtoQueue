macro(_set_cache VAR_NAME VAR_DESCRIPTION)
    set(${VAR_NAME} ${${VAR_NAME}} CACHE INTERNAL ${VAR_DESCRIPTION})
    message(STATUS "|  ${VAR_NAME}: ${${VAR_NAME}}")
endmacro()

message(STATUS "/================================================================================")
message(STATUS "| 3rdParty Configuration")
message(STATUS "|================================================================================")

# Code coverage

if(BUILD_PROTOQUEUE_TESTS AND GENERATE_COVERAGE)
    set(CMAKE_BUILD_TYPE COVERAGE CACHE INTERNAL "Coverage enabled build")
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(COVERAGE_FLAG "--coverage")
    endif()
    set(CMAKE_CXX_FLAGS_COVERAGE
        "-g -O0 ${COVERAGE_FLAG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE)
    set(CMAKE_C_FLAGS_COVERAGE
        "-g -O0 ${COVERAGE_FLAG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used for linking binaries during coverage builds."
        FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used by the shared libraries linker during coverage builds."
        FORCE)
    mark_as_advanced(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
endif()


# GTest configuration

if(BUILD_PROTOQUEUE_TESTS)
    message(STATUS "| googletest Configuration")
    message(STATUS "|================================================================================")
    if(USE_SYSTEM_GTEST)
        find_package(GTest)
    endif()
    if(NOT USE_SYSTEM_GTEST AND NOT GTEST_FOUND)
        add_subdirectory(googletest)
        cmake_policy(SET CMP0026 OLD)
        get_property(GTEST_LIBRARIES TARGET gtest PROPERTY LOCATION)
        get_property(GTEST_MAIN_LIBRARIES TARGET gtest_main PROPERTY LOCATION)
        set(GTEST_BOTH_LIBRARIES gtest gtest_main)
        set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
    endif()

    _set_cache(GTEST_LIBRARIES "Location of libgtest.")
    _set_cache(GTEST_MAIN_LIBRARIES "Location of libgtest-main.")
    _set_cache(GTEST_BOTH_LIBRARIES "Location of both gtest libraries.")
    _set_cache(GTEST_INCLUDE_DIRS "Location of gtest include files.")
    message(STATUS "|================================================================================")
endif()


# Protobuf configuration

if(BUILD_PROTOQUEUE_TESTS)
    message(STATUS "| Protobuf Configuration")
    message(STATUS "|================================================================================")
    if(DEFINED ENV{PROTOBUF_FOUND})
        # Allow setting the option through an environment variable
        set(PROTOBUF_FOUND $ENV{PROTOBUF_FOUND})
    endif()
    if(DEFINED ENV{PROTOBUF_LIBRARIES})
        # Allow setting the option through an environment variable
        set(PROTOBUF_LIBRARIES $ENV{PROTOBUF_LIBRARIES})
    endif()
    if(DEFINED ENV{PROTOBUF_INCLUDE_DIRS})
        # Allow setting the option through an environment variable
        set(PROTOBUF_INCLUDE_DIRS $ENV{PROTOBUF_INCLUDE_DIRS})
    endif()
    message(STATUS "Protobuf ${PROTOBUF_FOUND} ${PROTOBUF_LIBRARIES} ${PROTOBUF_INCLUDE_DIRS}")

    find_package(Protobuf REQUIRED)

    if(USE_SYSTEM_PROTOBUF)
        find_package(Protobuf)
    endif()
    if(NOT USE_SYSTEM_PROTOBUF OR NOT PROTOBUF_FOUND)
        message(FATAL_ERROR "System libprotobuf and protoc is required to proceed.\n"
            "If you're using OS X and have Homebrew installed, you can use the command:\n"
            "\tbrew install protobuf\n"
            "If you're using a Linux distribution like Ubuntu, you can use the command:\n"
            "\tapt-get install libprotobuf-dev protoc")
    endif()

    _set_cache(PROTOBUF_FOUND "Set if libprotobuf was found or built.")
    _set_cache(PROTOBUF_LIBRARIES "Location of libprotobuf.")
    _set_cache(PROTOBUF_INCLUDE_DIRS "Location of protobuf include files.")
    message(STATUS "|================================================================================")
endif()

message(STATUS "| zeromq Configuration")
message(STATUS "|================================================================================")
if(NOT ZEROMQ_FOUND)
    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "Do not build the tests for zeromq" FORCE)
    add_subdirectory(zeromq)
    set(ZEROMQ_LIBRARIES libzmq)
    set(ZEROMQ_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/zeromq/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq)
endif()

_set_cache(ZEROMQ_LIBRARIES "Location of libzmq.")
_set_cache(ZEROMQ_INCLUDE_DIRS "Location of gtest include files.")
message(STATUS "|================================================================================")
