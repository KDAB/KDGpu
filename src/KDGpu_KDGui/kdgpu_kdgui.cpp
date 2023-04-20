#include "kdgpu_kdgui.h"

#include <cstdlib>

namespace KDGpuKDGui {

std::string assetPath()
{
    const char *path = std::getenv("KDGPU_KDGUI_ASSET_PATH");
    if (path)
        return path;

#if defined(KDGPU_KDGUI_ASSET_PATH)
    return KDGPU_KDGUI_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpuKDGui
