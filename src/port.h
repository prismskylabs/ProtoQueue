#ifndef PROTOQUEUE_Port_H
#define PROTOQUEUE_Port_H

#include <ostream>


namespace prism {
namespace protoqueue {

class Port {
  public:
    Port(const int& port) : value{port} {}

    friend std::ostream& operator<<(std::ostream& os, const Port& port);

    int value;
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Port_H */
