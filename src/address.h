#ifndef PROTOQUEUE_ADDRESS_H
#define PROTOQUEUE_ADDRESS_H

#include <ostream>
#include <string>


class Address {
  public:
    Address(const std::string& address="") : value(std::string(address)) {}

    friend std::ostream& operator<<(std::ostream& os, const Address& address);

    std::string value;
};

#endif
