#include "type.h"

#include <ostream>


std::ostream& operator<<(std::ostream& os, const Type& type) {
    return os << type.value;
}
