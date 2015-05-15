#ifndef PQ_API_H
#define PQ_API_H

#include <string>
#include <iostream>

#include "protoqueue.h"


namespace pq {
    namespace priv {
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
    }

    // Bind methods
    template <typename T, typename... Args>
    ProtoQueue<T> Bind(Args&&... args) {
        ProtoQueue<T> queue;
        priv::set_option(queue, std::forward<Args>(args)...);
        queue.Bind();
        return queue;
    }

    // Connect methods
    template <typename T, typename... Args>
    ProtoQueue<T> Connect(Args&&... args) {
        ProtoQueue<T> queue;
        priv::set_option(queue, std::forward<Args>(args)...);
        queue.Connect();
        return queue;
    }
}

#endif
