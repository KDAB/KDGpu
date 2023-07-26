/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "imgui_input_handler.h"

#include <imgui.h>

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

    ImGuiIO &io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = mapSpecialKey(Key_Tab);
    io.KeyMap[ImGuiKey_LeftArrow] = mapSpecialKey(Key_Left);
    io.KeyMap[ImGuiKey_RightArrow] = mapSpecialKey(Key_Right);
    io.KeyMap[ImGuiKey_UpArrow] = mapSpecialKey(Key_Up);
    io.KeyMap[ImGuiKey_DownArrow] = mapSpecialKey(Key_Down);
    io.KeyMap[ImGuiKey_PageUp] = mapSpecialKey(Key_PageUp);
    io.KeyMap[ImGuiKey_PageDown] = mapSpecialKey(Key_PageDown);
    io.KeyMap[ImGuiKey_Home] = mapSpecialKey(Key_Home);
    io.KeyMap[ImGuiKey_End] = mapSpecialKey(Key_End);
    io.KeyMap[ImGuiKey_Insert] = mapSpecialKey(Key_Insert);
    io.KeyMap[ImGuiKey_Delete] = mapSpecialKey(Key_Delete);
    io.KeyMap[ImGuiKey_Backspace] = mapSpecialKey(Key_Backspace);
    io.KeyMap[ImGuiKey_Space] = Key_Space; // This is in the ASCII range of KDGui::key. No need to map.
    io.KeyMap[ImGuiKey_Enter] = mapSpecialKey(Key_Enter);
    io.KeyMap[ImGuiKey_Escape] = mapSpecialKey(Key_Escape);
    io.KeyMap[ImGuiKey_KeyPadEnter] = mapSpecialKey(Key_NumPad_Enter);

    // These ones allow ImGui to handle a few common shortcuts (cut/copy/paste/select all/undo/redo)
    io.KeyMap[ImGuiKey_A] = Key_A;
    io.KeyMap[ImGuiKey_C] = Key_C;
    io.KeyMap[ImGuiKey_V] = Key_V;
    io.KeyMap[ImGuiKey_X] = Key_X;
    io.KeyMap[ImGuiKey_Y] = Key_Y;
    io.KeyMap[ImGuiKey_Z] = Key_Z;

    // Default state is no keys pressed
    memset(m_keys, 0, sizeof(m_keys));
}

ImGuiInputHandler::~ImGuiInputHandler()
{
}

void ImGuiInputHandler::event(EventReceiver *target, Event *ev)
{
    switch (ev->type()) {
    case Event::Type::MousePress: {
        auto e = static_cast<MousePressEvent *>(ev);
        m_mouseButtons |= e->button();

        ImGuiIO &io = ImGui::GetIO();
        e->setAccepted(io.WantCaptureMouse);
        break;
    }

    case Event::Type::MouseRelease: {
        auto e = static_cast<MousePressEvent *>(ev);
        m_mouseButtons &= ~e->button();

        ImGuiIO &io = ImGui::GetIO();
        e->setAccepted(io.WantCaptureMouse);
        break;
    }

    case Event::Type::MouseMove: {
        auto e = static_cast<MouseMoveEvent *>(ev);
        m_mousePos[0] = e->xPos();
        m_mousePos[1] = e->yPos();
        e->setAccepted(false); // Don't prevent further event processing
        break;
    }

    case Event::Type::MouseWheel: {
        auto e = static_cast<MouseWheelEvent *>(ev);
        m_xWheelValue = static_cast<float>(e->xDelta()) / 120.0f;
        m_yWheelValue = static_cast<float>(e->yDelta()) / 120.0f;

        ImGuiIO &io = ImGui::GetIO();
        e->setAccepted(io.WantCaptureMouse);
        break;
    }

    case Event::Type::KeyPress:
    case Event::Type::KeyRelease: {
        auto e = static_cast<KeyEvent *>(ev);

        // Pass along the modifier state
        m_modifiers = e->modifiers();

        const auto isKeyPress = e->type() == Event::Type::KeyPress;
        const auto key = e->key();
        if (key < 256) {
            m_keys[key] = isKeyPress;
        } else if (key >= ms_firstSpecialKey && key <= ms_lastSpecialKey) {
            const int mappedKey = mapSpecialKey(key);
            m_keys[mappedKey] = isKeyPress;
        }

        ImGuiIO &io = ImGui::GetIO();
        e->setAccepted(io.WantCaptureKeyboard);

        break;
    }

    case Event::Type::TextInput: {
        auto e = static_cast<TextInputEvent *>(ev);
        m_capturedText = e->text();

        ImGuiIO &io = ImGui::GetIO();
        e->setAccepted(io.WantTextInput);

        break;
    }

    default: {
        break;
    }
    }

    Object::event(target, ev);
}

void ImGuiInputHandler::updateInputState()
{
    ImGuiIO &io = ImGui::GetIO();

    io.MousePos = ImVec2(static_cast<float>(m_mousePos[0]) * io.FontGlobalScale, static_cast<float>(m_mousePos[1]) * io.FontGlobalScale);
    io.MouseDown[0] = m_mouseButtons & LeftButton;
    io.MouseDown[1] = m_mouseButtons & RightButton;
    io.MouseDown[2] = m_mouseButtons & MiddleButton;
    io.MouseWheelH = m_xWheelValue;
    io.MouseWheel = m_yWheelValue;
    m_xWheelValue = 0.0f;
    m_yWheelValue = 0.0f;

    // Set keyboard modifier and key state
    memcpy(io.KeysDown, m_keys, sizeof(m_keys));
    io.KeyAlt = m_modifiers & Mod_Alt;
    io.KeyCtrl = m_modifiers & Mod_Control;
    io.KeyShift = m_modifiers & Mod_Shift;
    io.KeySuper = m_modifiers & Mod_Logo;

    // Set captured text state
    io.AddInputCharactersUTF8(m_capturedText.data());
    m_capturedText.clear();
}

} // namespace KDGpuExample
