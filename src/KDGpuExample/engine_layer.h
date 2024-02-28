/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>

#include <KDGpuExample/kdgpuexample_export.h>

namespace KDGpuExample {

class Engine;

/**
    @class EngineLayer
    @brief EngineLayer ...
    @ingroup kdgpuexample
    @headerfile engine_layer.h <KDGpuExample/engine_layer.h>
 */
class KDGPUEXAMPLE_EXPORT EngineLayer : public KDFoundation::Object
{
public:
    ~EngineLayer() override;

    const Engine *engine() const noexcept { return m_engine; }
    Engine *engine() noexcept { return m_engine; }

protected:
    virtual void onAttached();
    virtual void onDetached();
    virtual void update();

private:
    Engine *m_engine{ nullptr };

    friend class Engine;
};

} // namespace KDGpuExample
