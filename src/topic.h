#ifndef PROTOQUEUE_TOPIC_H
#define PROTOQUEUE_TOPIC_H

#include <ostream>
#include <string>


class Topic {
  public:
    Topic(const std::string& topic="") : value{topic} {}

    friend std::ostream& operator<<(std::ostream& os, const Topic& topic);

    std::string value;
};

#endif
