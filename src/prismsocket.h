#ifndef PROTOQUEUE_Socket_H
#define PROTOQUEUE_Socket_H


#include <thread>
#include <condition_variable>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <functional>
#include <string>
#include <deque>
#include <syslog.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>



#include <zmq.hpp>

#include "address.h"
#include "port.h"
#include "protocontext.h"
#include "topic.h"
#include "type.h"

#if !defined(gettid)
#	define  gettid()   syscall(SYS_gettid)
#endif

namespace prism {
namespace protoqueue {



template <class T>
class Socket
{
	/* This class constructor, destructor and methods shall NOT be used in multithreaded way.
	 * This object is not protected and not supposed to be.
	 * For sending socket it is required for all constructor, destructor and methods to be called
	 * from single thread.
	 * It is DESIGN PATTERN imposed by ZMQ (that we rely on underneath).
	 * Violating this pattern has implications.
	 *
	 */

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

    bool send(const T & object, bool block)
    {
        object.CheckInitialized();
        std::string output;
        object.SerializeToString(&output);
        zmq::message_t message{output.length()};
        memcpy(message.data(), output.data(), output.length());
        int flags = block ? 0: ZMQ_NOBLOCK;
        return socket_.send(message, flags);
    }

    T receive(bool block=true)
    {
        zmq::message_t message;
        T object;
        if ((block && socket_.recv(&message)) || socket_.recv(&message, ZMQ_NOBLOCK)) {
            object.ParseFromString(std::string{(char*) message.data(), message.size()});
        }
        return object;
    }

    void setOption(const Port& port) {
        port_ = port;
    }

    void setOption(const Address& address) {
        address_ = address;
    }

    void setOption(const Topic& topic) {
        topic_ = topic;
    }

    void setOption(const Type& type) {
        type_ = type;
    }

    Port getPort() { return port_; }
    Address getAddress() { return address_; }
    Type getType() { return type_; }

  protected:
    enum { MAX_MESSAGES_IN_FLIGHT = 4 };
    enum { MAX_ZMQ_LINGER_MS = 300 };

    void socketConnect4Send(zmq::socket_t& sock)
    {
    	/*  MAX_ZMQ_LINGER_MS can not be zero (as it is widely used sometimes).
    	 *  We quite likely lose the message if we set linger to zero
    	 *  It is because receiving side does polling with certain period.
    	 *  Linger value shall be more than time between polls on receiving side
    	 *  (which we typically hold here <= 100 ms).
    	 *  We do not want to have this linger time larger than it is now (300ms),
    	 *  because in this case we have issue with graceful closing of threads,
    	 *  as wait times there are about 500ms or 1 sec.
    	 *  300 ms shall be enough to send any message that we have ( < 2MB) on local host.
    	 */

        auto linger = MAX_ZMQ_LINGER_MS;
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
    using Socket<T>::MAX_ZMQ_LINGER_MS;


public:
    Bind(const Port & port, const Type & type)
        : Socket<T>(port, type)
    {

        auto linger = MAX_ZMQ_LINGER_MS;
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

// In most usecases you shall not use this class Connect<T>,
// but use Sender<T> class further below.
// Because using Connect tends people to do it against ZMQ design patterns.
// If you are not sure what you shall use, then use Sender.
template <class T>
class Connect: public Socket<T>
{
    using Socket<T>::address_;
    using Socket<T>::port_;
    using Socket<T>::type_;
    using Socket<T>::topic_;
    using Socket<T>::socket_;
    using Socket<T>::send;

public:
    Connect(const Port & port, const Type & type)
        : Socket<T>(port, type)
    {
    	Socket<T>::socketConnect4Send(socket_);
    }
};

template <class T, int MAXFIFOSIZE=100>
class PQSender
{
public:

	PQSender() = delete;
	PQSender(const PQSender& queue) = delete;
	PQSender& operator=(const PQSender& queue) = delete;

	PQSender(PQSender&& queue) = delete;
	PQSender& operator=(PQSender&& queue) = delete;

	PQSender(const Port & port, const Type & type)
		: port_(port), type_(type), stop_(false)
	{
	}

	~PQSender()
	{
		stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
    void send(const T & object)
    {
    	int lastSize = 0;
        object.CheckInitialized();
        {
        	std::lock_guard<std::mutex> grd(mutex_);
        	buf_.emplace_back(object);
        	lastSize = buf_.size();
        }
        condVar_.notify_one();
        if(lastSize > MAXFIFOSIZE)
		{
			// This is to prevent forgetting invoking sendWorker()
        	std::ostringstream oss;
        	oss << "Too many messages accumulated in prism::protoqueue::Sender."
					" sendWorker() is never invoked? MAXFIFOSIZE too low? "
        			"No receiving party? Receving party too slow?"
					<<+ "port = " << port_.value << " fifo buffer size = " << lastSize;
        	std::string msg = oss.str();
			syslog(LOG_WARNING, "%s", msg.c_str());
			fprintf(stderr, "%s\n", msg.c_str());
		}

    }

    // it is supposed to be called in separate thread and it will just run there
    // taking care of all sending
    void sendWorker()
    {
    	// ZMQ socket shall be var on stack of thread that sends.
    	// If you make it member of object not on stack of sending thread, it's wrong use
    	// See http://zeromq.org/whitepapers:0mq-termination
        Connect<T> socket(port_, type_);

    	for(;;)
    	{
    		if(stop_)
    		{
    			const char* msg = "Closing thread for sending messages as process stop was requested. ";
    			syslog(LOG_INFO, "%s", msg);
    			return;
    		}
			std::chrono::milliseconds waitTime = std::chrono::milliseconds(20);
			std::unique_lock<std::mutex> mlock(mutex_);
			if(condVar_.wait_for(mlock, waitTime) == std::cv_status::no_timeout)
			{
				std::vector<T> buf2;
				while (!buf_.empty())
				{
					buf2.emplace_back( buf_.front() );
					buf_.pop_front();
				}
				mlock.unlock();
				for ( T & msg : buf2)
				{
					sendWithHelper(socket, msg, __FILE__, __LINE__);
				}
			}
    	}

    }


    void stop()
    {
    	stop_ = true;
    }

    Port getPort() { return port_; }


private:
    /*
     * According to ZMQ docs. If ZMQ_PUSH -ed message can not reach
     * ZMQ_PULL receiver, send call will block.
     * If we do call as non blocking, call will fail with errno = EAGAIN and
     * we will receive back that message was not sent.
     * If there is no receiving party, call will fail sending.
     * So, if it fails, check why nobody is available on receiving side.
     * Below we try to go non-blocking route. In this case we explicitly
     * become aware that message can not be sent (unlike in blocking case).
     * We however, need to manage some lifecycle things ourselves, but
     * that's what we have to do anyway if we want to fit into small amount
     * of memory on embedded device.
     */
    void sendWithHelper(Socket<T>& sock, T& message, const char* file, int line)
    {
        int maxtry = 100;
    	int tryDelayMs = 50;
    	for (int i = 0; i < maxtry; i++)
    	{
    		bool block = false;
    		bool sent = sock.send(message, block);
    		if (sent)
    			return;

    		if(stop_)
    		{
    			const char* msg = "Giving up retrying sending message as process stop was requested. ";
    			syslog(LOG_WARNING, "%s", msg);
    			fprintf(stderr, "%s", msg);
    			break;
    		}

    		if(i > maxtry/2 && i % 10 == 1 ) // Complain only if we went above half of timeout
    		{
    			std::ostringstream oss;
    			oss << "Failed to send message via ZMQ queue. Attempt " << i << ". At " << file << ":" << line;
    			syslog(LOG_ERR, "%s", oss.str().c_str());
    			fprintf(stderr, "%s\n", oss.str().c_str());
    		}

    		std::this_thread::sleep_for(std::chrono::milliseconds(tryDelayMs));
    	}

    	std::ostringstream oss;
    	oss << "Failed to send message via ZMQ queue. " << maxtry << " attempts failed. at " << file << ":" << line;
    	syslog(LOG_ERR, "%s", oss.str().c_str());
    	fprintf(stderr, "%s", oss.str().c_str());
    	throw std::runtime_error(oss.str().c_str());
    }

private:
	std::mutex mutex_;
	std::condition_variable condVar_;
	std::deque<T> buf_;
    //std::function< bool() > shutdownRequested_;
    //std::function< void(std::string) > requestShutdown_;
	Port port_;
	Type type_;
    volatile bool stop_;



};


template<class T, int MAXFIFOSIZE=100>
class Sender
{
public:
	Sender() = delete;
	Sender(const Sender& queue) = delete;
	Sender& operator=(const Sender& queue) = delete;

	Sender(Sender&& queue) = delete;
	Sender& operator=(Sender&& queue) = delete;

	Sender(const Port & port, const Type & type)
		: sender_(port, type), failed_(false)
	{
		syslog(LOG_INFO, "In Sender constructor. Port = %d", (int)port.value);
		worker_ = std::move(std::thread(&Sender<T, MAXFIFOSIZE>::sendWorker, this));
	}

    void send(const T & object)
    {
    	sender_.send(object);
    }

    void stop()
    {
    	sender_.stop();
    }

    Port getPort() { return sender_.getPort(); }

	~Sender()
	{
		syslog(LOG_INFO, "In Sender destructor. Port = %d", (int)getPort().value);
		sender_.stop();
		worker_.join();
	}

	prism::protoqueue::PQSender<T, MAXFIFOSIZE>& getSender() { return sender_; }

private:
	prism::protoqueue::PQSender<T, MAXFIFOSIZE> sender_;
	std::thread worker_;
	bool failed_;
	std::string errMessage_;

	void sendWorker()
	{

	    const char* where = "prism::protoqueue::Sender<T>::sendWorker()";
	    try
	    {
	        syslog(LOG_INFO, "Started thread with %s %lx, port = %d ->", where, gettid(), getPort().value);
	        fprintf(stdout, "Started thread with %s %lx -> = port = %d\n", where, gettid(), getPort().value);
			sender_.sendWorker();
	        syslog(LOG_INFO, "<-%s() thread completed, port = %d", where, getPort().value);
	        fprintf(stdout, "<-%s() thread completed, port = %d\n", where, getPort().value);
	    }
	    catch(std::exception & e)
	    {
	    	char buf[1024];
	    	snprintf(buf, sizeof(buf), "Error \"%s\" in thread (%s [%lx]), port = %d", e.what(), where, gettid(), getPort().value);
	    	syslog(LOG_ERR,"%s", buf);
	    	fprintf(stderr, "%s\n", buf);
	    	failed_ = true;
	    	errMessage_ = std::string(buf);
	    }
	    catch(...)
	    {
	    	char buf[1024];
	    	snprintf(buf, sizeof(buf), "Unknown exception occur in thread (%s [%lx]), port = %d", where, gettid(), getPort().value);
	    	syslog(LOG_ERR,"%s", buf);
	    	fprintf(stderr, "%s\n", buf);
	    	failed_ = true;
	    	errMessage_ = std::string(buf);
	    }
	    syslog(LOG_INFO, "Leaving %s, port = %d", where, getPort().value);
	    fprintf(stdout, "Leaving %s, port = %d", where, getPort().value);
	}


};


} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Socket_H */
