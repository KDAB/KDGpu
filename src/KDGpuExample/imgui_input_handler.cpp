/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "imgui_input_handler.h"

#include <KDGui/gui_application.h>
#include <KDGui/abstract_clipboard.h>

namespace KDGpuExample {

using namespace KDFoundation;
using namespace KDGui;

ImGuiInputHandler::ImGuiInputHandler()
    : Object()
{
    setObjectName("ImGui Input Handler");

    // Set up key mapping
    //
    // The default is for KDGui::Key values under 256 to map directly to those indices in
    // our m_keys vector.
    //
    // For the special keys listed below, we will map these to entries above 256.
    //
    auto imguiContext = ImGui::GetCurrentContext();
    if (!imguiContext)
        ImGui::CreateContext();

    ImGui::GetIO().GetClipboardTextFn = [](void *) -> const char * {
        // We need to keep the clipboard buffer in memory, ImGui uses a reference to it
        static std::string currentClipboardString;
        if (AbstractClipboard *clipboard = KDGui::GuiApplication::instance()->guiPlatformIntegration()->clipboard()) {
            currentClipboardString = clipboard->text();
        }
        return currentClipboardString.c_str();
    };
}

void ImGuiInputHandler::event(EventReceiver *target, Event *ev)
{
    ImGuiIO &io = ImGui::GetIO();

    switch (ev->type()) {
    case Event::Type::MousePress: {
        auto e = static_cast<MousePressEvent *>(ev);
        e->setAccepted(io.WantCaptureMouse);
        if (io.WantCaptureMouse) {
            io.AddMouseButtonEvent(mapMouseButton(e->buttons()), true);
        }

        break;
    }

    case Event::Type::MouseRelease: {
        auto e = static_cast<MouseReleaseEvent *>(ev);
        e->setAccepted(io.WantCaptureMouse);
        if (io.WantCaptureMouse) {
            io.AddMouseButtonEvent(mapMouseButton(e->buttons()), false);
        }

        break;
    }

    case Event::Type::MouseMove: {
        auto e = static_cast<MouseMoveEvent *>(ev);
        e->setAccepted(false); // Don't prevent further event processing

        io.AddMousePosEvent(static_cast<float>(e->xPos()), static_cast<float>(e->yPos()));

        break;
    }

    case Event::Type::MouseWheel: {
        auto e = static_cast<MouseWheelEvent *>(ev);
        e->setAccepted(io.WantCaptureMouse);
        if (io.WantCaptureMouse) {
            const float xWheelValue = static_cast<float>(e->xDelta()) / 120.0f;
            const float yWheelValue = static_cast<float>(e->yDelta()) / 120.0f;

            io.AddMouseWheelEvent(xWheelValue, yWheelValue);
        }

        break;
    }

    case Event::Type::KeyPress:
    case Event::Type::KeyRelease: {
        auto e = static_cast<KeyEvent *>(ev);
        e->setAccepted(io.WantCaptureKeyboard);

        if (io.WantCaptureKeyboard) {
            const auto isKeyPress = e->type() == Event::Type::KeyPress;
            const auto key = e->key();
            const ImGuiKey mappedKey = mapKeyCode(key);
            if (mappedKey != ImGuiKey_None) {
                io.AddKeyEvent(mappedKey, isKeyPress);
            }
        }

        break;
    }

    case Event::Type::TextInput: {
        auto e = static_cast<TextInputEvent *>(ev);
        e->setAccepted(io.WantTextInput);

        if (io.WantTextInput) {
            io.AddInputCharactersUTF8(e->text().c_str());
        }

        break;
    }

    default: {
        break;
    }
    }

    Object::event(target, ev);
}

ImGuiMouseButton ImGuiInputHandler::mapMouseButton(const KDGui::MouseButtons button) const
{
    switch (button.toInt()) {
    case NoButton:
    case LeftButton:
        return ImGuiMouseButton_Left;
    case MiddleButton:
        return ImGuiMouseButton_Middle;
    case RightButton:
        return ImGuiMouseButton_Right;
    }

    return ImGuiMouseButton_Left;
}

ImGuiKey ImGuiInputHandler::mapKeyCode(const KDGui::Key key) const
{
    switch (key) {
    case KDGui::Key_Tab:
        return ImGuiKey_Tab;
    case KDGui::Key_Left:
        return ImGuiKey_LeftArrow;
    case KDGui::Key_Right:
        return ImGuiKey_RightArrow;
    case KDGui::Key_Up:
        return ImGuiKey_UpArrow;
    case KDGui::Key_Down:
        return ImGuiKey_DownArrow;
    case KDGui::Key_PageUp:
        return ImGuiKey_PageUp;
    case KDGui::Key_PageDown:
        return ImGuiKey_DownArrow;
    case KDGui::Key_Home:
        return ImGuiKey_Home;
    case KDGui::Key_End:
        return ImGuiKey_End;
    case KDGui::Key_Insert:
        return ImGuiKey_Insert;
    case KDGui::Key_Delete:
        return ImGuiKey_Delete;
    case KDGui::Key_Backspace:
        return ImGuiKey_Backspace;
    case KDGui::Key_Space:
        return ImGuiKey_Space;
    case KDGui::Key_Enter:
        return ImGuiKey_Enter;
    case KDGui::Key_Escape:
        return ImGuiKey_Escape;
    case KDGui::Key_NumPad_Enter:
        return ImGuiKey_Enter;
    case KDGui::Key_A:
        return ImGuiKey_A;
    case KDGui::Key_C:
        return ImGuiKey_C;
    case KDGui::Key_V:
        return ImGuiKey_V;
    case KDGui::Key_X:
        return ImGuiKey_X;
    case KDGui::Key_Y:
        return ImGuiKey_Y;
    case KDGui::Key_Z:
        return ImGuiKey_Z;
    case KDGui::Key_LeftControl:
        return ImGuiMod_Ctrl;
    case KDGui::Key_LeftShift:
        return ImGuiMod_Shift;
    default:
        break;
    }

    return ImGuiKey_None;
}

} // namespace KDGpuExample
