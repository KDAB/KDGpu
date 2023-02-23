#pragma once

#include <KDFoundation/object.h>

#include <toy_renderer_kdgui/toy_renderer_kdgui_export.h>

namespace ToyRendererKDGui {

class Engine;

class TOY_RENDERER_KDGUI_EXPORT EngineLayer : public KDFoundation::Object
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

} // namespace ToyRendererKDGui
