#ifndef PROTOQUEUE_Socket_H
#define PROTOQUEUE_Socket_H

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

template <class T>
class Socket
{
  public:
    Socket() = delete;
    Socket(const Socket& queue) = delete;
    Socket& operator=(const Socket& queue) = delete;

    Socket(Socket&& queue) = default;
    Socket& operator=(Socket&& queue) = default;

    Socket(const Port & port, const Type & type)
        : port_(port)
        , type_(type)
        , socket_(ProtoContext::Get().zmq, type.value)
    {

    }

    bool Send(const T & object)
    {
        object.CheckInitialized();
        std::string output;
        object.SerializeToString(&output);
        zmq::message_t message{output.length()};
        memcpy(message.data(), output.data(), output.length());
        zmq::socket_t theSocket(ProtoContext::Get().zmq, type_.value);
        return socket_.send(message);
    }

    T Receive(bool block=true)
    {
        zmq::message_t message;
        T object;
        if ((block && socket_.recv(&message)) || socket_.recv(&message, ZMQ_NOBLOCK)) {
            object.ParseFromString(std::string{(char*) message.data(), message.size()});
        }
        return object;
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

  protected:
    Port port_;
    Address address_;
    Topic topic_;
    Type type_;
    zmq::socket_t socket_;
};

template <class T>
class Bind: public Socket<T>
{
    using Socket<T>::address_;
    using Socket<T>::port_;
    using Socket<T>::type_;
    using Socket<T>::topic_;
    using Socket<T>::socket_;
public:
    Bind(const Port & port, const Type & type)
        : Socket<T>(port, type)
    {
        auto linger = 0;
        socket_.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        if (address_.value.empty())
        {
            if (port_.value == 0) {
                socket_.bind("tcp://127.0.0.1:*");
                char port_string[1024];
                size_t size = sizeof(port_string);
                socket_.getsockopt(ZMQ_LAST_ENDPOINT, &port_string, &size);
                address_.value = port_string;
                auto& address = address_.value;
                port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
            } else {
                try {
                    std::stringstream url;
                    url << "tcp://127.0.0.1:" << port_.value;
                    socket_.bind(url.str().data());
                    char port_string[1024];
                    size_t size = sizeof(port_string);
                    socket_.getsockopt(ZMQ_LAST_ENDPOINT, &port_string, &size);
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
                socket_.bind(address.data());
                port_.value = std::stoi(address.substr(address.find(':', 4) + 1, address.length()));
            } catch (zmq::error_t& e) {
                std::cerr << "Socket Error [" << e.num() << "]: " << e.what() << std::endl;
                throw e;
            }
        }
    }
};

template <class T>
class Connect: public Socket<T>
{
    using Socket<T>::address_;
    using Socket<T>::port_;
    using Socket<T>::type_;
    using Socket<T>::topic_;
    using Socket<T>::socket_;
public:
    Connect(const Port & port, const Type & type)
        : Socket<T>(port, type)
    {
        auto linger = 0;
        socket_.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        if (type_.value == ZMQ_SUB)
            socket_.setsockopt(ZMQ_SUBSCRIBE, topic_.value.data(), topic_.value.length());

        std::stringstream url;
        url << "tcp://127.0.0.1:" << port_.value;
        address_.value = url.str();
        socket_.connect(url.str().data());
    }
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Socket_H */
