cmake_minimum_required(VERSION 2.8.7)
project(protoqueue)

if(NOT ${CMAKE_VERSION} LESS 3.2)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "Checking compiler flags for C++11 support.")
    # Set C++11 support flags for various compilers
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "C++11 is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    elseif(COMPILER_SUPPORTS_CXX0X)
        message(STATUS "C++0x is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        endif()
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(PROTOQUEUE_LIBRARIES protoqueue CACHE INTERNAL "")
set(PROTOQUEUE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "")

macro(_declare_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()
    message(STATUS "|  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

message(STATUS "/================================================================================")
message(STATUS "| protoqueue Options")
message(STATUS "|================================================================================")
_declare_option(USE_SYSTEM_GTEST
    "If ON, this project will look in the system paths for an installed gtest library." OFF)
_declare_option(USE_SYSTEM_PROTOBUF
    "If ON, this project will look in the system paths for an installed protobuf library and compiler." ON)
_declare_option(BUILD_PROTOQUEUE_TESTS
    "If ON, this project will build the unit tests." ON)
_declare_option(GENERATE_COVERAGE
    "If ON, this project will generate coverage reports." OFF)

if(BUILD_PROTOQUEUE_TESTS)
    enable_testing()
endif()

add_subdirectory(3rdParty)
add_subdirectory(src)
if(BUILD_PROTOQUEUE_TESTS)
    add_subdirectory(test)
endif()
