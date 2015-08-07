#ifndef PROTOQUEUE_API_H
#define PROTOQUEUE_API_H

#include "socket.h"


namespace prism {
namespace protoqueue {

namespace detail {
    template <typename T>
    void set_option(T& queue) {
    }

    template <typename T, typename Arg>
    void set_option(T& queue, Arg&& arg) {
        queue.SetOption(std::forward<Arg>(arg));
    }

    template <typename T, typename Arg, typename... Args>
    void set_option(T& queue, Arg&& arg, Args&&... args) {
        set_option(queue, std::forward<Arg>(arg));
        set_option(queue, std::forward<Args>(args)...);
    }
} // namespace detail

// Bind methods
template <typename T, typename... Args>
Socket<T> Bind(Args&&... args) {
    Socket<T> queue;
    detail::set_option(queue, std::forward<Args>(args)...);
    queue.Bind();
    return queue;
}

// Connect methods
template <typename T, typename... Args>
Socket<T> Connect(Args&&... args) {
    Socket<T> queue;
    detail::set_option(queue, std::forward<Args>(args)...);
    queue.Connect();
    return queue;
}

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_API_H */
