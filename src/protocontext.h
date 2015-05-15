#ifndef PROTOCONTEXT_H
#define PROTOCONTEXT_H

#include <zmq.hpp>


class ProtoContext {
  public:
    static ProtoContext& Get() {
        static ProtoContext instance;
        return instance;
    }

    zmq::context_t zmq{1};

  private:
    ProtoContext() {};
};

#endif
