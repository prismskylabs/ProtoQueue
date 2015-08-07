#include "address.h"

#include <ostream>


namespace prism {
namespace protoqueue {

std::ostream& operator<<(std::ostream& os, const Address& address) {
    return os << address.value;
}

} // namespace protoqueue
} // namespace prism
