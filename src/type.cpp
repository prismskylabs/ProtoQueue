#include "type.h"

#include <ostream>


namespace prism {
namespace protoqueue {

std::ostream& operator<<(std::ostream& os, const Type& type) {
    return os << type.value;
}

} // namespace protoqueue
} // namespace prism
