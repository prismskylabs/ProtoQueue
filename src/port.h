#ifndef PROTOQUEUE_Port_H
#define PROTOQUEUE_Port_H

#include <ostream>
#include <stdint.h>
#include <stdexcept>


namespace prism {
namespace protoqueue {

class Port {
  public:
    Port(const int port)
    {
        if (port< 0)
               throw std::runtime_error("Socket port must be not less 0");

        if (port > 0xffff)
               throw std::runtime_error("Socket port can't exceed 0xffff");
        value = static_cast<uint16_t>(port);
    }

    friend std::ostream& operator<<(std::ostream& os, const Port& port);

    std::uint16_t  value;
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Port_H */
