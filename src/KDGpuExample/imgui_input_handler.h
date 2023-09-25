/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>

#include <KDGui/gui_events.h>
#include <KDGui/kdgui_keys.h>

#include <kdbindings/property.h>

#include <KDGpuExample/kdgpuexample_export.h>

#include <imgui.h>

namespace KDGpuExample {

/**
    @class ImGuiInputHandler
    @brief ImGuiInputHandler ...
    @ingroup kdgpuexample
    @headerfile imgui_input_handler.h <KDGpuExample/imgui_input_handler.h>
 */
class KDGPUEXAMPLE_EXPORT ImGuiInputHandler : public KDFoundation::Object
{
public:
    KDBindings::Property<bool> enabled{ true };

    ImGuiInputHandler();

    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

private:
    ImGuiMouseButton mapMouseButton(KDGui::MouseButton button) const;
    ImGuiKey mapKeyCode(KDGui::Key key) const;
};

} // namespace KDGpuExample
