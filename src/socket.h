#ifndef PROTOQUEUE_Socket_H
#define PROTOQUEUE_Socket_H

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <zmq.hpp>

#include "address.h"
#include "port.h"
#include "protocontext.h"
#include "topic.h"
#include "type.h"


namespace prism {
namespace protoqueue {

template <typename T>
class ProtoQueue {
  using socket_ptr = std::unique_ptr<zmq::socket_t, std::function<void(zmq::socket_t*)>>;

  public:
    ProtoQueue() : address_{}, port_{0}, type_{ZMQ_PAIR} {}
    ProtoQueue(const ProtoQueue& queue) = delete;
    ProtoQueue& operator=(const ProtoQueue& queue) = delete;
    ProtoQueue(ProtoQueue&& queue)
            : port_(queue.port_), address_(queue.address_), topic_(queue.topic_), type_(queue.type_) {
        socket_ = std::move(queue.socket_);
    }
    ProtoQueue& operator=(ProtoQueue&& queue) {
        socket_ = std::move(queue.socket_);
        port_ = queue.port_;
        address_ = queue.address_;
        topic_ = queue.topic_;
        type_ = queue.type_;
        return *this;
    }

    void Send(T t) {
        t.CheckInitialized();
        std::string output;
        t.SerializeToString(&output);
        zmq::message_t message{output.length()};
        memcpy(message.data(), output.data(), output.length());
        socket_->send(message);
    }

    T Receive(bool block=true) {
        zmq::message_t message;
        T t;
        if ((block && socket_->recv(&message)) || socket_->recv(&message, ZMQ_NOBLOCK)) {
            t.ParseFromString(std::string{(char*) message.data(), message.size()});
        }
        return t;
    }

    void Bind() {
        auto close = [] (zmq::socket_t* socket) {
            socket->close();
            delete socket;
        };
        socket_ = socket_ptr(new zmq::socket_t{ProtoContext::Get().zmq, type_.value}, close);
        if (address_.value.empty()) {
            if (port_.value == 0) {
                socket_->bind("tcp://*:*");
                char port_string[1024];
                size_t size = sizeof(port_string);
                socket_->getsockopt(ZMQ_LAST_ENDPOINT, &port_string, &size);
                address_.value = port_string;
                auto& address = address_.value;
                port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
            } else {
                try {
                    std::stringstream url;
                    url << "tcp://0.0.0.0:" << port_.value;
                    socket_->bind(url.str().data());
                    char port_string[1024];
                    size_t size = sizeof(port_string);
                    socket_->getsockopt(ZMQ_LAST_ENDPOINT, &port_string, &size);
                    address_.value = port_string;
                    auto& address = address_.value;
                    port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
                } catch (zmq::error_t& e) {
                    std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
                    throw e;
                }
            }
        } else {
            try {
                auto& address = address_.value;
                socket_->bind(address.data());
                port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
            } catch (zmq::error_t& e) {
                std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
                throw e;
            }
        }
    }

    void Connect() {
        if (port_.value <= 0) {
            throw std::runtime_error("Socket port must be above 0");
        }
        auto close = [] (zmq::socket_t* socket) {
            socket->close();
            delete socket;
        };
        socket_ = socket_ptr(new zmq::socket_t{ProtoContext::Get().zmq, type_.value}, close);
        if (type_.value == ZMQ_SUB) {
            socket_->setsockopt(ZMQ_SUBSCRIBE, topic_.value.data(), topic_.value.length());
        }
        std::stringstream url;
        url << "tcp://0.0.0.0:" << port_.value;
        address_.value = url.str();
        socket_->connect(url.str().data());
    }

    void SetOption(const Port& port) {
        port_ = port;
    }

    void SetOption(const Address& address) {
        address_ = address;
    }

    void SetOption(const Topic& topic) {
        topic_ = topic;
    }

    void SetOption(const Type& type) {
        type_ = type;
    }

    Port get_port() { return port_; }
    Address get_address() { return address_; }
    Type get_type() { return type_; }

  private:
    socket_ptr socket_;
    Port port_;
    Address address_;
    Topic topic_;
    Type type_;
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Socket_H */
