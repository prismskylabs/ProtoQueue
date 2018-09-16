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
    bool Send(const T & object, bool block)
    {
        object.CheckInitialized();
        std::string output;
        object.SerializeToString(&output);
        zmq::message_t message{output.length()};
        memcpy(message.data(), output.data(), output.length());
        zmq::socket_t sock(ProtoContext::Get().zmq, type_.value);
        socketConnect4Send(sock);
        int flags = block ? 0: ZMQ_NOBLOCK;
        return sock.send(message, flags);
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
    enum { MAX_MESSAGES_IN_FLIGHT = 4 };
    void socketConnect4Send(zmq::socket_t& sock)
    {
        auto linger = 0;
        sock.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        if (type_.value == ZMQ_SUB)
            sock.setsockopt(ZMQ_SUBSCRIBE, topic_.value.data(), topic_.value.length());

        int hwm = MAX_MESSAGES_IN_FLIGHT;
        sock.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));

        std::stringstream url;
        url << "tcp://127.0.0.1:" << port_.value;
        address_.value = url.str();
        sock.connect(url.str().data());
    }


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
    using Socket<T>::MAX_MESSAGES_IN_FLIGHT;
public:
    Bind(const Port & port, const Type & type)
        : Socket<T>(port, type)
    {
        auto linger = 0;
        socket_.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        int hwm = MAX_MESSAGES_IN_FLIGHT;
        if(type.value == ZMQ_PUSH)
        	socket_.setsockopt(ZMQ_SNDHWM, &hwm, sizeof(hwm));
        if(type.value == ZMQ_PULL)
        	socket_.setsockopt(ZMQ_RCVHWM, &hwm, sizeof(hwm));

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
    }
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Socket_H */
