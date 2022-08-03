#pragma once

#include "adapter.h"
#include "serenity_gpu_global.h"

#include <toy_renderer/handle.h>

#include <string>
#include <vector>

namespace Gpu {

struct Instance_t;

struct InstanceOptions {
    std::string applicationName{ "Serenity Application" };
    uint32_t applicationVersion{ SERENITY_MAKE_API_VERSION(0, 1, 0, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class Instance
{
public:
    Instance();
    Instance(const InstanceOptions &options);
    ~Instance();

    bool isValid() const { return m_handle.isValid(); }

    Adapter requestAdapter(const AdapterSettings &settings = AdapterSettings());

private:
    Handle<Instance_t> m_handle;
};

} // namespace Gpu
