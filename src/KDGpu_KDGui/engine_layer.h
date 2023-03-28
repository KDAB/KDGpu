#pragma once

#include <KDFoundation/object.h>

#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

namespace KDGpuKDGui {

class Engine;

class KDGPU_KDGUI_EXPORT EngineLayer : public KDFoundation::Object
{
public:
    ~EngineLayer() override;

    const Engine *engine() const noexcept { return m_engine; }

protected:
    virtual void onAttached();
    virtual void onDetached();
    virtual void update();

private:
    Engine *m_engine{ nullptr };

    friend class Engine;
};

} // namespace KDGpuKDGui
