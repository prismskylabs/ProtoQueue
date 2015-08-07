#ifndef PROTOQUEUE_Type_H
#define PROTOQUEUE_Type_H

#include <ostream>


class Type {
  public:
    Type(const int& type) : value{type} {}

    friend std::ostream& operator<<(std::ostream& os, const Type& type);

    int value;
};

#endif /* PROTOQUEUE_Type_H */
