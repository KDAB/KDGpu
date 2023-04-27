#include "kdgpuexample.h"

#include <cstdlib>

namespace KDGpuExample {

std::string assetPath()
{
    const char *path = std::getenv("KDGPUEXAMPLE_ASSET_PATH");
    if (path)
        return path;

#if defined(KDGPUEXAMPLE_ASSET_PATH)
    return KDGPUEXAMPLE_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDGpuExample
