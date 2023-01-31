#pragma once

#include <Serenity/core/object.h>

class Engine;

class EngineLayer : public Serenity::Object
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
