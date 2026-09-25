#pragma once
#include "Event.h"
#include "KeyCode.h"
#include "OS.h"

namespace jug
{

// ===========================================
//  System (Raw SDL Event)
// ===========================================

class SystemEvent : public Event
{
public:
    explicit SystemEvent(const SDL_Event& _event);

    [[nodiscard]] const SDL_Event& GetEvent() const;
    [[nodiscard]] String           ToString() const override;
    JUG_EVENT_BODY(SystemEvent);

private:
    const SDL_Event* m_pEvent = nullptr;
};

// ===========================================
//  Application
// ===========================================

class QuitEvent : public Event
{
public:
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(QuitEvent);
};

class LowMemoryEvent : public Event
{
public:
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(LowMemoryEvent);
};

// ================================================================
//  Window
// ================================================================

class WindowEvent : public Event
{
public:
    explicit WindowEvent(SDL_WindowID _wndId);
    [[nodiscard]] SDL_WindowID GetWindow() const;

protected:
    SDL_WindowID m_wndID = {};
};

class WindowShownEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowShownEvent);
};

class WindowHiddenEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowHiddenEvent);
};

class WindowExposedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowExposedEvent);
};

class WindowOccludedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowOccludedEvent);
};

class WindowMinimizedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowMinimizedEvent);
};

class WindowMaximizedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowMaximizedEvent);
};

class WindowRestoredEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowRestoredEvent);
};

class WindowMouseEnterEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowMouseEnterEvent);
};

class WindowMouseLeaveEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowMouseLeaveEvent);
};

class WindowFocusGainedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowFocusGainedEvent);
};

class WindowFocusLostEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowFocusLostEvent);
};

class WindowCloseRequestedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowCloseRequestedEvent);
};

class WindowDestroyedEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowDestroyedEvent);
};

class WindowEnterFullscreenEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowEnterFullscreenEvent);
};

class WindowLeaveFullscreenEvent : public WindowEvent
{
public:
    using WindowEvent::WindowEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowLeaveFullscreenEvent);
};

class WindowMovedEvent : public WindowEvent
{
public:
    WindowMovedEvent(SDL_WindowID _wndId, int32_t _x, int32_t _y);

    [[nodiscard]] int32_t GetX() const;
    [[nodiscard]] int32_t GetY() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowMovedEvent);

private:
    int32_t m_x = 0;
    int32_t m_y = 0;
};

class WindowResizedEvent : public WindowEvent
{
public:
    WindowResizedEvent(SDL_WindowID _wndId, int32_t _width, int32_t _height);

    [[nodiscard]] int32_t GetWidth() const;
    [[nodiscard]] int32_t GetHeight() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowResizedEvent);

private:
    int32_t m_width  = 0;
    int32_t m_height = 0;
};

class WindowPixelSizeChangedEvent : public WindowEvent
{
public:
    WindowPixelSizeChangedEvent(SDL_WindowID _wndId, int32_t _pixelWidth, int32_t _pixelHeight);

    [[nodiscard]] int32_t GetPixelWidth() const;
    [[nodiscard]] int32_t GetPixelHeight() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowPixelSizeChangedEvent);

private:
    int32_t m_pixelWidth  = 0;
    int32_t m_pixelHeight = 0;
};

class WindowDisplayScaleChangedEvent : public WindowEvent
{
public:
    WindowDisplayScaleChangedEvent(SDL_WindowID _wndId, float _scale);

    [[nodiscard]] float GetScale() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowDisplayScaleChangedEvent);

private:
    float m_scale = 1.f;
};

class WindowDisplayChangedEvent : public WindowEvent
{
public:
    WindowDisplayChangedEvent(SDL_WindowID _wndId, uint32_t _displayId);

    [[nodiscard]] uint32_t GetDisplayID() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(WindowDisplayChangedEvent);

private:
    uint32_t m_displayId = 0;
};

// ===========================================
//  Display
// ===========================================

class DisplayEvent : public Event
{
public:
    explicit DisplayEvent(uint32_t _displayId);

    [[nodiscard]] uint32_t GetDisplayID() const;

private:
    uint32_t m_displayId = 0;
};

class DisplayAddedEvent : public DisplayEvent
{
public:
    using DisplayEvent::DisplayEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DisplayAddedEvent);
};

class DisplayRemovedEvent : public DisplayEvent
{
public:
    using DisplayEvent::DisplayEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DisplayRemovedEvent);
};

class DisplayOrientationChangedEvent : public DisplayEvent
{
public:
    DisplayOrientationChangedEvent(uint32_t _displayId, int32_t _orientation);

    [[nodiscard]] int32_t GetOrientation() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DisplayOrientationChangedEvent);

private:
    int32_t m_orientation = 0;
};

class DisplayContentScaleChangedEvent : public DisplayEvent
{
public:
    DisplayContentScaleChangedEvent(uint32_t _displayId, float _scale);

    [[nodiscard]] float GetScale() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DisplayContentScaleChangedEvent);

private:
    float m_scale = 1.f;
};

// ===========================================
//  Keyboard
// ===========================================

class KeyDownEvent : public Event
{
public:
    KeyDownEvent(SDL_WindowID _wndId, eKey _key, Flags<eKeyMod> _mods, bool _bRepeat);

    [[nodiscard]] SDL_WindowID   GetWindow() const;
    [[nodiscard]] eKey           GetKey() const;
    [[nodiscard]] Flags<eKeyMod> GetMods() const;
    [[nodiscard]] bool           IsRepeat() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(KeyDownEvent);

private:
    SDL_WindowID   m_wndID   = {};
    eKey           m_key     = eKey::None;
    Flags<eKeyMod> m_mods    = {};
    bool           m_bRepeat = false;
};

class KeyUpEvent : public Event
{
public:
    KeyUpEvent(SDL_WindowID _wndId, eKey _key, Flags<eKeyMod> _mods);

    [[nodiscard]] SDL_WindowID   GetWindow() const;
    [[nodiscard]] eKey           GetKey() const;
    [[nodiscard]] Flags<eKeyMod> GetMods() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(KeyUpEvent);

private:
    SDL_WindowID   m_wndID = {};
    eKey           m_key   = eKey::None;
    Flags<eKeyMod> m_mods  = {};
};

class TextInputEvent : public Event
{
public:
    TextInputEvent(SDL_WindowID _wndId, StringView _text);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] StringView   GetText() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(TextInputEvent);

private:
    SDL_WindowID m_wndID = {};
    StringView   m_text  = {};
};

class TextEditingEvent : public Event
{
public:
    TextEditingEvent(SDL_WindowID _wndId, StringView _text, int32_t _start, int32_t _length);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] StringView   GetText() const;
    [[nodiscard]] int32_t      GetStart() const;
    [[nodiscard]] int32_t      GetLength() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(TextEditingEvent);

private:
    SDL_WindowID m_wndID  = {};
    StringView   m_text   = {};
    int32_t      m_start  = 0;
    int32_t      m_length = 0;
};

// ===========================================
//  Mouse
// ===========================================

class MouseMovedEvent : public Event
{
public:
    MouseMovedEvent(SDL_WindowID _wndId, float _x, float _y, float _deltaX, float _deltaY);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] float        GetX() const;
    [[nodiscard]] float        GetY() const;
    [[nodiscard]] float        GetDeltaX() const;
    [[nodiscard]] float        GetDeltaY() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(MouseMovedEvent);

private:
    SDL_WindowID m_wndID  = {};
    float        m_x      = 0.f;
    float        m_y      = 0.f;
    float        m_deltaX = 0.f;
    float        m_deltaY = 0.f;
};

class MouseButtonDownEvent : public Event
{
public:
    MouseButtonDownEvent(SDL_WindowID _wndId, eMouse _button, float _x, float _y, int32_t _clicks);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] eMouse       GetButton() const;
    [[nodiscard]] float        GetX() const;
    [[nodiscard]] float        GetY() const;
    [[nodiscard]] int32_t      GetClicks() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(MouseButtonDownEvent);

private:
    SDL_WindowID m_wndID  = {};
    eMouse       m_button = eMouse::None;
    float        m_x      = 0.f;
    float        m_y      = 0.f;
    int32_t      m_clicks = 0;
};

class MouseButtonUpEvent : public Event
{
public:
    MouseButtonUpEvent(SDL_WindowID _wndId, eMouse _button, float _x, float _y);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] eMouse       GetButton() const;
    [[nodiscard]] float        GetX() const;
    [[nodiscard]] float        GetY() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(MouseButtonUpEvent);

private:
    SDL_WindowID m_wndID  = {};
    eMouse       m_button = eMouse::None;
    float        m_x      = 0.f;
    float        m_y      = 0.f;
};

class MouseWheelEvent : public Event
{
public:
    MouseWheelEvent(SDL_WindowID _wndId, float _scrollX, float _scrollY, float _x, float _y);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] float        GetScrollX() const;
    [[nodiscard]] float        GetScrollY() const;
    [[nodiscard]] float        GetX() const;
    [[nodiscard]] float        GetY() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(MouseWheelEvent);

private:
    SDL_WindowID m_wndID   = {};
    float        m_scrollX = 0.f;
    float        m_scrollY = 0.f;
    float        m_x       = 0.f;
    float        m_y       = 0.f;
};

// ===========================================
//  Gamepad
// ===========================================

class GamepadEvent : public Event
{
public:
    explicit GamepadEvent(uint32_t _joystickId);

    [[nodiscard]] uint32_t GetJoystickID() const;

private:
    uint32_t m_joystickId = 0;
};

class GamepadAddedEvent : public GamepadEvent
{
public:
    using GamepadEvent::GamepadEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(GamepadAddedEvent);
};

class GamepadRemovedEvent : public GamepadEvent
{
public:
    using GamepadEvent::GamepadEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(GamepadRemovedEvent);
};

class GamepadButtonDownEvent : public GamepadEvent
{
public:
    GamepadButtonDownEvent(uint32_t _joystickId, eGamepadButton _button);

    [[nodiscard]] eGamepadButton GetButton() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(GamepadButtonDownEvent);

private:
    eGamepadButton m_button = {};
};

class GamepadButtonUpEvent : public GamepadEvent
{
public:
    GamepadButtonUpEvent(uint32_t _joystickId, eGamepadButton _button);

    [[nodiscard]] eGamepadButton GetButton() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(GamepadButtonUpEvent);

private:
    eGamepadButton m_button = {};
};

class GamepadAxisMotionEvent : public GamepadEvent
{
public:
    GamepadAxisMotionEvent(uint32_t _joystickId, eGamepadAxis _axis, float _value);

    [[nodiscard]] eGamepadAxis GetAxis() const;
    [[nodiscard]] float        GetValue() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(GamepadAxisMotionEvent);

private:
    eGamepadAxis m_axis  = {};
    float        m_value = 0.f;
};

// ===========================================
//  Drop
// ===========================================

class DropEvent : public Event
{
public:
    DropEvent(SDL_WindowID _wndId, float _x, float _y);

    [[nodiscard]] SDL_WindowID GetWindow() const;
    [[nodiscard]] float        GetX() const;
    [[nodiscard]] float        GetY() const;

protected:
    SDL_WindowID m_wndID = {};
    float        m_x     = 0.f;
    float        m_y     = 0.f;
};

class DropBeginEvent : public DropEvent
{
public:
    using DropEvent::DropEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DropBeginEvent);
};

class DropCompleteEvent : public DropEvent
{
public:
    using DropEvent::DropEvent;
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DropCompleteEvent);
};

class DropFileEvent : public DropEvent
{
public:
    DropFileEvent(SDL_WindowID _wndId, float _x, float _y, StringView _path);

    [[nodiscard]] StringView GetPath() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DropFileEvent);

private:
    StringView m_path = {};
};

class DropTextEvent : public DropEvent
{
public:
    DropTextEvent(SDL_WindowID _wndId, float _x, float _y, StringView _text);

    [[nodiscard]] StringView GetText() const;

    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(DropTextEvent);

private:
    StringView m_text = {};
};

// ===========================================
//  Clipboard
// ===========================================

class ClipboardUpdatedEvent : public Event
{
public:
    [[nodiscard]] String ToString() const override;
    JUG_EVENT_BODY(ClipboardUpdatedEvent);
};

}   // namespace jug
