#ifndef PROTOQUEUE_ProtoContext_H
#define PROTOQUEUE_ProtoContext_H

#include <zmq.hpp>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#define gettid() syscall(SYS_gettid)
#include <syslog.h>

namespace prism {
namespace protoqueue {


class ProtoContext {
  public:
    static ProtoContext& Get();

    zmq::context_t zmq{1};

    ProtoContext(const ProtoContext &) = delete;
  private:
    ~ProtoContext() {syslog(LOG_INFO,"Zmq destroyed %lx %p", gettid(), this);};
    ProtoContext() {syslog(LOG_INFO,"Zmq created %lx %p", gettid(), this);};
};

} // namespace prism
} // namespace protoqueue

#endif /* PROTOQUEUE_ProtoContext_H */
