#include "pch.h"
#include "ApplicationEvent.h"

#include <SDL3/SDL_keyboard.h>

#include "StringFormat.h"

namespace jug
{

// ===========================================
//  App
// ===========================================

String QuitEvent::ToString() const
{
    return "QuitEvent";
}

String LowMemoryEvent::ToString() const
{
    return "LowMemoryEvent";
}

SystemEvent::SystemEvent(
    const SDL_Event& _event)
    : m_pEvent(&_event)
{
}

const SDL_Event& SystemEvent::GetEvent() const
{
    return *m_pEvent;
}

String SystemEvent::ToString() const
{
    return Format("SystemEvent: type: {}", m_pEvent->type);
}

// ===========================================
//  Window
// ===========================================

WindowEvent::WindowEvent(
    const SDL_WindowID _wndId)
    : m_wndID(_wndId)
{
}

SDL_WindowID WindowEvent::GetWindow() const
{
    return m_wndID;
}

String WindowShownEvent::ToString() const
{
    return Format("WindowShownEvent: window: {}", m_wndID);
}

String WindowHiddenEvent::ToString() const
{
    return Format("WindowHiddenEvent: window: {}", m_wndID);
}

String WindowExposedEvent::ToString() const
{
    return Format("WindowExposedEvent: window: {}", m_wndID);
}

String WindowOccludedEvent::ToString() const
{
    return Format("WindowOccludedEvent: window: {}", m_wndID);
}

String WindowMinimizedEvent::ToString() const
{
    return Format("WindowMinimizedEvent: window: {}", m_wndID);
}

String WindowMaximizedEvent::ToString() const
{
    return Format("WindowMaximizedEvent: window: {}", m_wndID);
}

String WindowRestoredEvent::ToString() const
{
    return Format("WindowRestoredEvent: window: {}", m_wndID);
}

String WindowMouseEnterEvent::ToString() const
{
    return Format("WindowMouseEnterEvent: window: {}", m_wndID);
}

String WindowMouseLeaveEvent::ToString() const
{
    return Format("WindowMouseLeaveEvent: window: {}", m_wndID);
}

String WindowFocusGainedEvent::ToString() const
{
    return Format("WindowFocusGainedEvent: window: {}", m_wndID);
}

String WindowFocusLostEvent::ToString() const
{
    return Format("WindowFocusLostEvent: window: {}", m_wndID);
}

String WindowCloseRequestedEvent::ToString() const
{
    return Format("WindowCloseRequestedEvent: window: {}", m_wndID);
}

String WindowDestroyedEvent::ToString() const
{
    return Format("WindowDestroyedEvent: window: {}", m_wndID);
}

String WindowEnterFullscreenEvent::ToString() const
{
    return Format("WindowEnterFullscreenEvent: window: {}", m_wndID);
}

String WindowLeaveFullscreenEvent::ToString() const
{
    return Format("WindowLeaveFullscreenEvent: window: {}", m_wndID);
}

WindowMovedEvent::WindowMovedEvent(
    const SDL_WindowID _wndId,
    const int32_t      _x,
    const int32_t      _y)
    : WindowEvent(_wndId)
    , m_x(_x)
    , m_y(_y)
{
}

int32_t WindowMovedEvent::GetX() const
{
    return m_x;
}

int32_t WindowMovedEvent::GetY() const
{
    return m_y;
}

String WindowMovedEvent::ToString() const
{
    return Format("WindowMovedEvent: window: {}, x: {}, y: {}", m_wndID, m_x, m_y);
}

WindowResizedEvent::WindowResizedEvent(
    const SDL_WindowID _wndId,
    const int32_t      _width,
    const int32_t      _height)
    : WindowEvent(_wndId)
    , m_width(_width)
    , m_height(_height)
{
}

int32_t WindowResizedEvent::GetWidth() const
{
    return m_width;
}

int32_t WindowResizedEvent::GetHeight() const
{
    return m_height;
}

String WindowResizedEvent::ToString() const
{
    return Format("WindowResizedEvent: window: {}, width: {}, height: {}", m_wndID, m_width, m_height);
}

WindowPixelSizeChangedEvent::WindowPixelSizeChangedEvent(
    const SDL_WindowID _wndId,
    const int32_t      _pixelWidth,
    const int32_t      _pixelHeight)
    : WindowEvent(_wndId)
    , m_pixelWidth(_pixelWidth)
    , m_pixelHeight(_pixelHeight)
{
}

int32_t WindowPixelSizeChangedEvent::GetPixelWidth() const
{
    return m_pixelWidth;
}

int32_t WindowPixelSizeChangedEvent::GetPixelHeight() const
{
    return m_pixelHeight;
}

String WindowPixelSizeChangedEvent::ToString() const
{
    return Format("WindowPixelSizeChangedEvent: window: {}, pixelWidth: {}, pixelHeight: {}", m_wndID, m_pixelWidth, m_pixelHeight);
}

WindowDisplayScaleChangedEvent::WindowDisplayScaleChangedEvent(
    const SDL_WindowID _wndId,
    const float        _scale)
    : WindowEvent(_wndId)
    , m_scale(_scale)
{
}

float WindowDisplayScaleChangedEvent::GetScale() const
{
    return m_scale;
}

String WindowDisplayScaleChangedEvent::ToString() const
{
    return Format("WindowDisplayScaleChangedEvent: window: {}, scale: {}", m_wndID, m_scale);
}

WindowDisplayChangedEvent::WindowDisplayChangedEvent(
    const SDL_WindowID _wndId,
    const uint32_t     _displayId)
    : WindowEvent(_wndId)
    , m_displayId(_displayId)
{
}

uint32_t WindowDisplayChangedEvent::GetDisplayID() const
{
    return m_displayId;
}

String WindowDisplayChangedEvent::ToString() const
{
    return Format("WindowDisplayChangedEvent: window: {}, display: {}", m_wndID, m_displayId);
}

// ===========================================
//  Display
// ===========================================

DisplayEvent::DisplayEvent(
    const uint32_t _displayId)
    : m_displayId(_displayId)
{
}

uint32_t DisplayEvent::GetDisplayID() const
{
    return m_displayId;
}

String DisplayAddedEvent::ToString() const
{
    return Format("DisplayAddedEvent: display: {}", GetDisplayID());
}

String DisplayRemovedEvent::ToString() const
{
    return Format("DisplayRemovedEvent: display: {}", GetDisplayID());
}

DisplayOrientationChangedEvent::DisplayOrientationChangedEvent(
    const uint32_t _displayId,
    const int32_t  _orientation)
    : DisplayEvent(_displayId)
    , m_orientation(_orientation)
{
}

int32_t DisplayOrientationChangedEvent::GetOrientation() const
{
    return m_orientation;
}

String DisplayOrientationChangedEvent::ToString() const
{
    return Format("DisplayOrientationChangedEvent: display: {}, orientation: {}", GetDisplayID(), m_orientation);
}

DisplayContentScaleChangedEvent::DisplayContentScaleChangedEvent(
    const uint32_t _displayId,
    const float    _scale)
    : DisplayEvent(_displayId)
    , m_scale(_scale)
{
}

float DisplayContentScaleChangedEvent::GetScale() const
{
    return m_scale;
}

String DisplayContentScaleChangedEvent::ToString() const
{
    return Format("DisplayContentScaleChangedEvent: display: {}, scale: {}", GetDisplayID(), m_scale);
}

// ===========================================
//  Keyboard
// ===========================================

KeyDownEvent::KeyDownEvent(
    const SDL_WindowID   _wndId,
    const eKey           _key,
    const Flags<eKeyMod> _mods,
    const bool           _bRepeat)
    : m_wndID(_wndId)
    , m_key(_key)
    , m_mods(_mods)
    , m_bRepeat(_bRepeat)
{
}

SDL_WindowID KeyDownEvent::GetWindow() const
{
    return m_wndID;
}

eKey KeyDownEvent::GetKey() const
{
    return m_key;
}

Flags<eKeyMod> KeyDownEvent::GetMods() const
{
    return m_mods;
}

bool KeyDownEvent::IsRepeat() const
{
    return m_bRepeat;
}

String KeyDownEvent::ToString() const
{
    return Format("KeyDownEvent: window: {}, key: {}, mods: {:#x}, repeat: {}", m_wndID, NameOf(m_key), m_mods.GetFlags(), m_bRepeat);
}

KeyUpEvent::KeyUpEvent(
    const SDL_WindowID   _wndId,
    const eKey           _key,
    const Flags<eKeyMod> _mods)
    : m_wndID(_wndId)
    , m_key(_key)
    , m_mods(_mods)
{
}

SDL_WindowID KeyUpEvent::GetWindow() const
{
    return m_wndID;
}

eKey KeyUpEvent::GetKey() const
{
    return m_key;
}

Flags<eKeyMod> KeyUpEvent::GetMods() const
{
    return m_mods;
}

String KeyUpEvent::ToString() const
{
    return Format("KeyUpEvent: window: {}, key: {}, mods: {:#x}", m_wndID, NameOf(m_key), m_mods.GetFlags());
}

TextInputEvent::TextInputEvent(
    const SDL_WindowID _wndId,
    const StringView   _text)
    : m_wndID(_wndId)
    , m_text(_text)
{
}

SDL_WindowID TextInputEvent::GetWindow() const
{
    return m_wndID;
}

StringView TextInputEvent::GetText() const
{
    return m_text;
}

String TextInputEvent::ToString() const
{
    return Format("TextInputEvent: window: {}, text: {}", m_wndID, m_text);
}

TextEditingEvent::TextEditingEvent(
    const SDL_WindowID _wndId,
    const StringView   _text,
    const int32_t      _start,
    const int32_t      _length)
    : m_wndID(_wndId)
    , m_text(_text)
    , m_start(_start)
    , m_length(_length)
{
}

SDL_WindowID TextEditingEvent::GetWindow() const
{
    return m_wndID;
}

StringView TextEditingEvent::GetText() const
{
    return m_text;
}

int32_t TextEditingEvent::GetStart() const
{
    return m_start;
}

int32_t TextEditingEvent::GetLength() const
{
    return m_length;
}

String TextEditingEvent::ToString() const
{
    return Format("TextEditingEvent: window: {}, text: {}, start: {}, length: {}", m_wndID, m_text, m_start, m_length);
}

// ===========================================
//  Mouse
// ===========================================

MouseMovedEvent::MouseMovedEvent(
    const SDL_WindowID _wndId,
    const float        _x,
    const float        _y,
    const float        _deltaX,
    const float        _deltaY)
    : m_wndID(_wndId)
    , m_x(_x)
    , m_y(_y)
    , m_deltaX(_deltaX)
    , m_deltaY(_deltaY)
{
}

SDL_WindowID MouseMovedEvent::GetWindow() const
{
    return m_wndID;
}

float MouseMovedEvent::GetX() const
{
    return m_x;
}

float MouseMovedEvent::GetY() const
{
    return m_y;
}

float MouseMovedEvent::GetDeltaX() const
{
    return m_deltaX;
}

float MouseMovedEvent::GetDeltaY() const
{
    return m_deltaY;
}

String MouseMovedEvent::ToString() const
{
    return Format("MouseMovedEvent: window: {}, x: {}, y: {}, deltaX: {}, deltaY: {}", m_wndID, m_x, m_y, m_deltaX, m_deltaY);
}

MouseButtonDownEvent::MouseButtonDownEvent(
    const SDL_WindowID _wndId,
    const eMouse       _button,
    const float        _x,
    const float        _y,
    const int32_t      _clicks)
    : m_wndID(_wndId)
    , m_button(_button)
    , m_x(_x)
    , m_y(_y)
    , m_clicks(_clicks)
{
}

SDL_WindowID MouseButtonDownEvent::GetWindow() const
{
    return m_wndID;
}

eMouse MouseButtonDownEvent::GetButton() const
{
    return m_button;
}

float MouseButtonDownEvent::GetX() const
{
    return m_x;
}

float MouseButtonDownEvent::GetY() const
{
    return m_y;
}

int32_t MouseButtonDownEvent::GetClicks() const
{
    return m_clicks;
}

String MouseButtonDownEvent::ToString() const
{
    return Format("MouseButtonDownEvent: window: {}, button: {}, x: {}, y: {}, clicks: {}", m_wndID, NameOf(m_button), m_x, m_y, m_clicks);
}

MouseButtonUpEvent::MouseButtonUpEvent(
    const SDL_WindowID _wndId,
    const eMouse       _button,
    const float        _x,
    const float        _y)
    : m_wndID(_wndId)
    , m_button(_button)
    , m_x(_x)
    , m_y(_y)
{
}

SDL_WindowID MouseButtonUpEvent::GetWindow() const
{
    return m_wndID;
}

eMouse MouseButtonUpEvent::GetButton() const
{
    return m_button;
}

float MouseButtonUpEvent::GetX() const
{
    return m_x;
}

float MouseButtonUpEvent::GetY() const
{
    return m_y;
}

String MouseButtonUpEvent::ToString() const
{
    return Format("MouseButtonUpEvent: window: {}, button: {}, x: {}, y: {}", m_wndID, NameOf(m_button), m_x, m_y);
}

MouseWheelEvent::MouseWheelEvent(
    const SDL_WindowID _wndId,
    const float        _scrollX,
    const float        _scrollY,
    const float        _x,
    const float        _y)
    : m_wndID(_wndId)
    , m_scrollX(_scrollX)
    , m_scrollY(_scrollY)
    , m_x(_x)
    , m_y(_y)
{
}

SDL_WindowID MouseWheelEvent::GetWindow() const
{
    return m_wndID;
}

float MouseWheelEvent::GetScrollX() const
{
    return m_scrollX;
}

float MouseWheelEvent::GetScrollY() const
{
    return m_scrollY;
}

float MouseWheelEvent::GetX() const
{
    return m_x;
}

float MouseWheelEvent::GetY() const
{
    return m_y;
}

String MouseWheelEvent::ToString() const
{
    return Format("MouseWheelEvent: window: {}, scrollX: {}, scrollY: {}, x: {}, y: {}", m_wndID, m_scrollX, m_scrollY, m_x, m_y);
}

// ===========================================
//  Gamepad
// ===========================================

GamepadEvent::GamepadEvent(
    const uint32_t _joystickId)
    : m_joystickId(_joystickId)
{
}

uint32_t GamepadEvent::GetJoystickID() const
{
    return m_joystickId;
}

String GamepadAddedEvent::ToString() const
{
    return Format("GamepadAddedEvent: joystick: {}", GetJoystickID());
}

String GamepadRemovedEvent::ToString() const
{
    return Format("GamepadRemovedEvent: joystick: {}", GetJoystickID());
}

GamepadButtonDownEvent::GamepadButtonDownEvent(
    const uint32_t       _joystickId,
    const eGamepadButton _button)
    : GamepadEvent(_joystickId)
    , m_button(_button)
{
}

eGamepadButton GamepadButtonDownEvent::GetButton() const
{
    return m_button;
}

String GamepadButtonDownEvent::ToString() const
{
    return Format("GamepadButtonDownEvent: joystick: {}, button: {}", GetJoystickID(), NameOf(m_button));
}

GamepadButtonUpEvent::GamepadButtonUpEvent(
    const uint32_t       _joystickId,
    const eGamepadButton _button)
    : GamepadEvent(_joystickId)
    , m_button(_button)
{
}

eGamepadButton GamepadButtonUpEvent::GetButton() const
{
    return m_button;
}

String GamepadButtonUpEvent::ToString() const
{
    return Format("GamepadButtonUpEvent: joystick: {}, button: {}", GetJoystickID(), NameOf(m_button));
}

GamepadAxisMotionEvent::GamepadAxisMotionEvent(
    const uint32_t     _joystickId,
    const eGamepadAxis _axis,
    const float        _value)
    : GamepadEvent(_joystickId)
    , m_axis(_axis)
    , m_value(_value)
{
}

eGamepadAxis GamepadAxisMotionEvent::GetAxis() const
{
    return m_axis;
}

float GamepadAxisMotionEvent::GetValue() const
{
    return m_value;
}

String GamepadAxisMotionEvent::ToString() const
{
    return Format("GamepadAxisMotionEvent: joystick: {}, axis: {}, value: {}", GetJoystickID(), NameOf(m_axis), m_value);
}

// ===========================================
//  Drop
// ===========================================

DropEvent::DropEvent(
    const SDL_WindowID _wndId,
    const float        _x,
    const float        _y)
    : m_wndID(_wndId)
    , m_x(_x)
    , m_y(_y)
{
}

SDL_WindowID DropEvent::GetWindow() const
{
    return m_wndID;
}

float DropEvent::GetX() const
{
    return m_x;
}

float DropEvent::GetY() const
{
    return m_y;
}

String DropBeginEvent::ToString() const
{
    return Format("DropBeginEvent: window: {}, x: {}, y: {}", m_wndID, GetX(), GetY());
}

String DropCompleteEvent::ToString() const
{
    return Format("DropCompleteEvent: window: {}, x: {}, y: {}", m_wndID, GetX(), GetY());
}

DropFileEvent::DropFileEvent(
    const SDL_WindowID _wndId,
    const float        _x,
    const float        _y,
    const StringView   _path)
    : DropEvent(_wndId, _x, _y)
    , m_path(_path)
{
}

StringView DropFileEvent::GetPath() const
{
    return m_path;
}

String DropFileEvent::ToString() const
{
    return Format("DropFileEvent: window: {}, x: {}, y: {}, path: {}", m_wndID, GetX(), GetY(), m_path);
}

DropTextEvent::DropTextEvent(
    const SDL_WindowID _wndId,
    const float        _x,
    const float        _y,
    const StringView   _text)
    : DropEvent(_wndId, _x, _y)
    , m_text(_text)
{
}

StringView DropTextEvent::GetText() const
{
    return m_text;
}

String DropTextEvent::ToString() const
{
    return Format("DropTextEvent: window: {}, x: {}, y: {}, text: {}", m_wndID, GetX(), GetY(), m_text);
}

// ===========================================
//  Clipboard
// ===========================================

String ClipboardUpdatedEvent::ToString() const
{
    return "ClipboardUpdatedEvent";
}

}   // namespace jug
