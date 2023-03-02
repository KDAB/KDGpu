#pragma once

namespace ToyRenderer {

struct ApiFence {
    virtual void wait() = 0;
    virtual void reset() = 0;
};

} // namespace ToyRenderer
