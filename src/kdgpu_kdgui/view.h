#pragma once

#include <KDGui/window.h>

#include <kdgpu/surface.h>
#include <kdgpu/surface_options.h>

#include <kdgpu_kdgui/kdgpu_kdgui_export.h>

namespace KDGpu {
class Instance;
}

namespace KDGpuKDGui {

class KDGPU_KDGUI_EXPORT View : public KDGui::Window
{
public:
    View();
    ~View();

    static KDGpu::SurfaceOptions surfaceOptions(KDGui::Window *w);
    KDGpu::Surface createSurface(KDGpu::Instance &instance);
};

} // namespace KDGpuKDGui
