#pragma once

#include <KDFoundation/object.h>

#include <KDGpuExample/kdgpuexample_export.h>

namespace KDGpuExample {

class Engine;

class KDGPUEXAMPLE_EXPORT EngineLayer : public KDFoundation::Object
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

} // namespace KDGpuExample
