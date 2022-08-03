#pragma once

#include <toy_renderer/handle.h>

#include <string>
#include <vector>

namespace Gpu {

struct Adapter_t;

struct AdapterSettings {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class Adapter
{
public:
    ~Adapter();

    bool isValid() const { return m_handle.isValid(); }

    const AdapterSettings &settings() { return m_settings; }

private:
    Adapter();

    Handle<Adapter_t> m_handle;
    AdapterSettings m_settings;

    friend class Instance;
};

} // namespace Gpu
