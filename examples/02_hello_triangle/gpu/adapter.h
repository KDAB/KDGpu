#pragma once

#include "adapter_features.h"
#include "adapter_properties.h"

#include <toy_renderer/handle.h>

#include <string>
#include <vector>

namespace Gpu {

struct Adapter_t;
struct Instance_t;

struct AdapterOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class Adapter
{
public:
    ~Adapter();

    bool isValid() const { return m_adapter.isValid(); }

private:
    explicit Adapter(const Handle<Instance_t> &instance, uint32_t adapterIndex);

    Handle<Adapter_t> m_adapter;

    friend class Instance;
};

} // namespace Gpu
