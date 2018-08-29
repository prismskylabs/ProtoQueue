#ifndef PROTOQUEUE_ProtoContext_H
#define PROTOQUEUE_ProtoContext_H

#include <zmq.hpp>

namespace prism {
namespace protoqueue {


class ProtoContext {
  public:
    static ProtoContext& Get();

    zmq::context_t zmq{1};

    ProtoContext(const ProtoContext &) = delete;
  private:
    ProtoContext() = default;
};

} // namespace prism
} // namespace protoqueue

#endif /* PROTOQUEUE_ProtoContext_H */
