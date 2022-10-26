#pragma once

#include <toy_renderer/handle.h>

#include <vector>

namespace ToyRenderer {

struct Adapter_t;

struct ApiInstance {
    virtual std::vector<Handle<Adapter_t>> queryAdapters() = 0;
};

} // namespace ToyRenderer
