#include "address.h"

#include <ostream>


std::ostream& operator<<(std::ostream& os, const Address& address) {
    return os << address.value;
}
