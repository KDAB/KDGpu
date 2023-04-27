#pragma once

#include <KDGui/window.h>

#include <KDGpu/surface.h>
#include <KDGpu/surface_options.h>

#include <KDGpuExample/kdgpuexample_export.h>

namespace KDGpu {
class Instance;
}

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT View : public KDGui::Window
{
public:
    View();
    ~View();

    static KDGpu::SurfaceOptions surfaceOptions(KDGui::Window *w);
    KDGpu::Surface createSurface(KDGpu::Instance &instance);
};

} // namespace KDGpuExample
