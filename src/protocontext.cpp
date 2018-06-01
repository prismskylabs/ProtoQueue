#include "protocontext.h"

namespace prism {
namespace protoqueue {


ProtoContext& ProtoContext::Get()
{
    static ProtoContext instance;
    return instance;
}

}
} // namespace protoqueue

