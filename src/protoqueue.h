#ifndef PROTOQUEUE_H
#define PROTOQUEUE_H

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <iostream>

#include <zmq.hpp>

#include "address.h"
#include "port.h"
#include "protocontext.h"
#include "topic.h"
#include "type.h"


template <typename T>
class ProtoQueue {
  using socket_ptr = std::unique_ptr<zmq::socket_t, std::function<void(zmq::socket_t*)>>;

  public:
    ProtoQueue() : address_{}, port_{0}, type_{ZMQ_PAIR} {}

    void Send(T t) {
        t.CheckInitialized();
        std::ostringstream stream;
        t.SerializeToOstream(&stream);
        auto str = stream.str();
        zmq::message_t message{str.length()};
        memcpy(message.data(), str.data(), str.length());
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
                try {
                    socket_->bind("tcp://*:*");
                } catch (zmq::error_t& e) {
                    std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
                }
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
                } catch (zmq::error_t& e) {
                    std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
                }
            }
        } else {
            try {
                auto& address = address_.value;
                socket_->bind(address.data());
                port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
            } catch (zmq::error_t& e) {
                std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
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
            std::cout << "setting sockopt" << std::endl;
            socket_->setsockopt(ZMQ_SUBSCRIBE, topic_.value.data(), topic_.value.length());
        }
        try {
            std::stringstream url;
            url << "tcp://0.0.0.0:" << port_.value;
            address_.value = url.str();
            socket_->connect(url.str().data());
        } catch (zmq::error_t& e) {
            std::cerr << "Error connecting socket [" << e.num() << "]: " << e.what() << std::endl;
        }
    }

    void SetOption(const Port& port) {
        std::cout << "Setting port " << port << std::endl;
        port_ = port;
    }

    void SetOption(const Address& address) {
        std::cout << "Setting address " << address << std::endl;
        address_ = address;
    }

    void SetOption(const Topic& topic) {
        std::cout << "Setting topic " << topic << std::endl;
        topic_ = topic;
    }

    void SetOption(const Type& type) {
        std::cout << "Setting type " << type << std::endl;
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

#endif
