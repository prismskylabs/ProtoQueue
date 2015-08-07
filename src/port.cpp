#include "port.h"

#include <ostream>


namespace prism {
namespace protoqueue {

std::ostream& operator<<(std::ostream& os, const Port& port) {
    return os << port.value;
}

} // namespace protoqueue
} // namespace prism
