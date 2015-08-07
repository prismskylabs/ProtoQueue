#ifndef PROTOQUEUE_Address_H
#define PROTOQUEUE_Address_H

#include <ostream>
#include <string>


namespace prism {
namespace protoqueue {

class Address {
  public:
    Address(const std::string& address="") : value(std::string(address)) {}

    friend std::ostream& operator<<(std::ostream& os, const Address& address);

    std::string value;
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Address_H */
