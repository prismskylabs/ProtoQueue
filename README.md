# ProtoQueue [![Build Status](https://travis-ci.org/prismskylabs/ProtoQueue.svg?branch=master)](https://travis-ci.org/prismskylabs/ProtoQueue) [![Coverage Status](https://coveralls.io/repos/prismskylabs/ProtoQueue/badge.svg)](https://coveralls.io/r/prismskylabs/ProtoQueue)

ProtoQueue is a template wrapper around [zeromq](https://github.com/zeromq/libzmq) that simplifies sending and receiving [protobuf](https://github.com/google/protobuf) messages. It abstracts away the messiness of context and port management, allowing you to maintain focus on the semantics of your network topology and message protocol.

Look at how easy it is to pass this message around:

```protobuf
// basic.proto
message Basic {
  required string value = 1;
}
```

```c++
#include <protoqueue.h> // This is the API header
#include <iostream>
#include "basic.pb.h"

int main(int argc, char** argv) {
    // This chooses a port for you and binds a ZMQ_PAIR to it by default
    auto sender = prism::protoqueue::Bind<Basic>();

    // This connects to the same port with a ZMQ_PAIR socket
    auto receiver = prism::protoqueue::Connect<Basic>(sender.get_port()); 

    Basic basic;
    basic.set_value("hello world!");
    sender.Send(basic);

    auto received = receiver.Receive();
    std::cout << received.value() << std::endl; // Prints "hello world!"
}
```

Without ProtoQueue, you're stuck with this [mess](https://gist.github.com/whoshuu/09b7e4c98938c043706f).

## Requirements

* A C++11 compatible compiler such as a suitably recent version of [clang](http://clang.llvm.org/) or [gcc](https://gcc.gnu.org/)
* [CMake](https://github.com/Kitware/CMake)
* [Google protobuf](https://github.com/google/protobuf)

## Build

The recommended way to build this library is by using an out of source build directory:

```shell
mkdir build
cd build
cmake ..
make
```

By default, this will build the tests. To run the tests, simply run:

```
ctest
```

This project has an embedded source of the googletest framework so it is not explicitly required on your system. To use your system version, set this CMake option:

```
cmake -DUSE_SYSTEM_GTEST=ON ..
```

and then run `make`.

A successful build will result in a single library that you can link against your project.

## Contributing

Please fork this repository and contribute back using [pull requests](https://github.com/prismskylabs/ProtoQueue/pulls). Features can be requested using [issues](https://github.com/prismskylabs/ProtoQueue/issues).
