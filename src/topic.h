#ifndef PROTOQUEUE_Topic_H
#define PROTOQUEUE_Topic_H

#include <ostream>
#include <string>


namespace prism {
namespace protoqueue {

class Topic {
  public:
    Topic(const std::string& topic="") : value(std::string(topic)) {}

    friend std::ostream& operator<<(std::ostream& os, const Topic& topic);

    std::string value;
};

} // namespace protoqueue
} // namespace prism

#endif /* PROTOQUEUE_Topic_H */
