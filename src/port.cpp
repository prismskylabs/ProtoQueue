#include "port.h"

#include <ostream>


std::ostream& operator<<(std::ostream& os, const Port& port) {
    return os << port.value;
}
