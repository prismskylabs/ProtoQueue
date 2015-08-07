#ifndef PROTOQUEUE_ProtoContext_H
#define PROTOQUEUE_ProtoContext_H

#include <zmq.hpp>


namespace prism {
namespace protoqueue {

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

} // namespace prism
} // namespace protoqueue

#endif /* PROTOQUEUE_ProtoContext_H */
