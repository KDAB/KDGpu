#include "adapter.h"

#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

Adapter::Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter)
    : m_api(api)
    , m_adapter(adapter)
{
}

Adapter::~Adapter()
{
}

} // namespace ToyRenderer
