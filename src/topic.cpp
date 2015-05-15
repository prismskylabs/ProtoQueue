#include "topic.h"

#include <ostream>


std::ostream& operator<<(std::ostream& os, const Topic& topic) {
    return os << topic.value;
}
