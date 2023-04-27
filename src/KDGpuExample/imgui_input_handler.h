#pragma once

#include <KDFoundation/object.h>

#include <KDGui/gui_events.h>
#include <KDGui/kdgui_keys.h>

#include <kdbindings/property.h>

#include <KDGpuExample/kdgpuexample_export.h>

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT ImGuiInputHandler : public KDFoundation::Object
{
public:
    KDBindings::Property<bool> enabled{ true };

    ImGuiInputHandler();
    ~ImGuiInputHandler() override;

    void updateInputState();
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;

private:
    int mapSpecialKey(KDGui::Key key) const
    {
        return 256 + static_cast<int>(key) - ms_firstSpecialKey;
    }

    int32_t m_mousePos[2];
    KDGui::MouseButtons m_mouseButtons{ KDGui::NoButton };
    float m_yWheelValue{ 0.0f };
    float m_xWheelValue{ 0.0f };
    bool m_keys[512];
    KDGui::KeyboardModifiers m_modifiers{ KDGui::Mod_NoModifiers };
    std::string m_capturedText;

    static constexpr int ms_firstSpecialKey{ KDGui::Key_Escape };
    static constexpr int ms_lastSpecialKey{ KDGui::Key_Menu };
};

} // namespace KDGpuExample
