#include "topic.h"

#include <ostream>


namespace prism {
namespace protoqueue {

std::ostream& operator<<(std::ostream& os, const Topic& topic) {
    return os << topic.value;
}

} // namespace protoqueue
} // namespace prism
