#ifndef PROTOQUEUE_Port_H
#define PROTOQUEUE_Port_H

#include <ostream>


class Port {
  public:
    Port(const int& port) : value{port} {}

    friend std::ostream& operator<<(std::ostream& os, const Port& port);

    int value;
};

#endif /* PROTOQUEUE_Port_H */
