#include "pch.h"
#include "Application.h"

#include "CoreLogger.h"
#include "EventDispatcher.h"
#include "Fatal.h"
#include "OS.h"

namespace jug
{

namespace
{
    Application* g_pSingleton = nullptr;

    constexpr float kGamepadAxisScale = 1.f / 32767.f;

    [[nodiscard]] Flags<eKeyMod> MakeKeyMods_(
        const SDL_Keymod _mod)
    {
        Flags<eKeyMod> mods = kZeroFlag;
        if (_mod & SDL_KMOD_SHIFT)
        {
            mods |= eKeyMod::Shift;
        }
        if (_mod & SDL_KMOD_CTRL)
        {
            mods |= eKeyMod::Ctrl;
        }
        if (_mod & SDL_KMOD_ALT)
        {
            mods |= eKeyMod::Alt;
        }
        if (_mod & SDL_KMOD_GUI)
        {
            mods |= eKeyMod::Gui;
        }
        if (_mod & SDL_KMOD_NUM)
        {
            mods |= eKeyMod::Num;
        }
        if (_mod & SDL_KMOD_CAPS)
        {
            mods |= eKeyMod::Caps;
        }
        if (_mod & SDL_KMOD_SCROLL)
        {
            mods |= eKeyMod::Scroll;
        }
        if (_mod & SDL_KMOD_MODE)
        {
            mods |= eKeyMod::Mode;
        }
        return mods;
    }
}   // namespace

Application::Application(
    const ApplicationDesc& _desc)
    : m_desc(_desc)
{
    JUG_ASSERT(!g_pSingleton, "Application instance already exists.");
    g_pSingleton = this;
}

void Application::Init()
{
}

void Application::Shutdown()
{
}

void Application::OnEvent(Event& _event)
{
}

void Application::Update(
    const float _deltaTimeSec)
{
}

Application::~Application()
{
    JUG_ASSERT(g_pSingleton == this, "Application instance mismatch.");
    g_pSingleton = nullptr;
}

Application& Application::GetSingleton()
{
    JUG_ASSERT(g_pSingleton, "Application instance is not created yet.");
    return *g_pSingleton;
}

int Application::Run(
    const int _argc,
    char**    _argv)
{
    m_cmdArgs.reserve(static_cast<size_t>(_argc));
    for (int i = 0; i < _argc; ++i)
    {
        m_cmdArgs.emplace_back(_argv[i]);
    }

    InitSystems_();
    Init();

    m_bRunning   = true;
    m_returnCode = 0;
    m_timer.Start();

    while (m_bRunning)
    {
        PollEvents_();
        if (!m_bRunning)
        {
            break;
        }

        m_timer.Lap();
        m_deltaTimeSec = TimeCastF(m_timer.GetElapsedCount(), eTimeUnit::Sec);
        Update(m_deltaTimeSec);
    }

    Shutdown();
    ShutdownSystems_();
    return m_returnCode;
}

void Application::Quit()
{
    m_bRunning = false;
}

// ===========================================
//  Time
// ===========================================

float Application::GetDeltaTimeSec() const
{
    return m_deltaTimeSec;
}

Span<const String> Application::GetCommandLineArgs() const
{
    return m_cmdArgs;
}

const ApplicationDesc& Application::GetDesc() const
{
    return m_desc;
}

void Application::SetReturnCode(
    const int _code)
{
    m_returnCode = _code;
}

void Application::InitSystems_() const
{
    const String appName    = String { m_desc.appName };
    const String appVersion = String { m_desc.appVersion };
    const String appIdent   = String { m_desc.appIdentifier };
    if (!SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appIdent.empty() ? nullptr : appIdent.c_str()))
    {
        JUG_CORE_LOG_WARN("SDL_SetAppMetadata failed: {}", SDL_GetError());
    }

    SDL_InitFlags flags = SDL_INIT_VIDEO;
    if (m_desc.bInitGamepad)
    {
        flags |= SDL_INIT_GAMEPAD;
    }

    if (!SDL_Init(flags))
    {
        JUG_FATAL("SDL_Init failed: {}", SDL_GetError());
    }
}

void Application::ShutdownSystems_() const
{
    SDL_Quit();
}

void Application::PollEvents_()
{
    SDL_Event msg;
    while (SDL_PollEvent(&msg))
    {
        SystemEvent sdlEvent { msg };
        DispatchEvent(sdlEvent);

        switch (msg.type)
        {
            case SDL_EVENT_QUIT:
            {
                QuitEvent event;
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_LOW_MEMORY:
            {
                LowMemoryEvent event;
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_SHOWN:
            {
                WindowShownEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_HIDDEN:
            {
                WindowHiddenEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_EXPOSED:
            {
                WindowExposedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_OCCLUDED:
            {
                WindowOccludedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_MINIMIZED:
            {
                WindowMinimizedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
            {
                WindowMaximizedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_RESTORED:
            {
                WindowRestoredEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            {
                WindowMouseEnterEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            {
                WindowMouseLeaveEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            {
                WindowFocusGainedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
            {
                WindowFocusLostEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            {
                WindowCloseRequestedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_DESTROYED:
            {
                WindowDestroyedEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            {
                WindowEnterFullscreenEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            {
                WindowLeaveFullscreenEvent event { msg.window.windowID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_MOVED:
            {
                WindowMovedEvent event { msg.window.windowID, msg.window.data1, msg.window.data2 };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_RESIZED:
            {
                WindowResizedEvent event { msg.window.windowID, msg.window.data1, msg.window.data2 };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            {
                WindowPixelSizeChangedEvent event { msg.window.windowID, msg.window.data1, msg.window.data2 };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            {
                const SDL_Window*              pWindow = SDL_GetWindowFromID(msg.window.windowID);
                const float                    scale   = pWindow ? SDL_GetWindowDisplayScale(SDL_GetWindowFromID(msg.window.windowID)) : 1.f;
                WindowDisplayScaleChangedEvent event { msg.window.windowID, scale };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            {
                WindowDisplayChangedEvent event { msg.window.windowID, static_cast<uint32_t>(msg.window.data1) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DISPLAY_ADDED:
            {
                DisplayAddedEvent event { msg.display.displayID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DISPLAY_REMOVED:
            {
                DisplayRemovedEvent event { msg.display.displayID };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DISPLAY_ORIENTATION:
            {
                DisplayOrientationChangedEvent event { msg.display.displayID, msg.display.data1 };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
            {
                DisplayContentScaleChangedEvent event { msg.display.displayID, SDL_GetDisplayContentScale(msg.display.displayID) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_KEY_DOWN:
            {
                KeyDownEvent event { msg.key.windowID, static_cast<eKey>(msg.key.scancode), MakeKeyMods_(msg.key.mod), msg.key.repeat };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_KEY_UP:
            {
                KeyUpEvent event { msg.key.windowID, static_cast<eKey>(msg.key.scancode), MakeKeyMods_(msg.key.mod) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_TEXT_INPUT:
            {
                TextInputEvent event { msg.text.windowID, msg.text.text ? StringView { msg.text.text } : StringView {} };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_TEXT_EDITING:
            {
                TextEditingEvent event { msg.edit.windowID, msg.edit.text ? StringView { msg.edit.text } : StringView {}, msg.edit.start, msg.edit.length };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_MOUSE_MOTION:
            {
                MouseMovedEvent event { msg.motion.windowID, msg.motion.x, msg.motion.y, msg.motion.xrel, msg.motion.yrel };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                MouseButtonDownEvent event { msg.button.windowID, static_cast<eMouse>(msg.button.button), msg.button.x, msg.button.y, msg.button.clicks };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                MouseButtonUpEvent event { msg.button.windowID, static_cast<eMouse>(msg.button.button), msg.button.x, msg.button.y };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_MOUSE_WHEEL:
            {
                MouseWheelEvent event { msg.wheel.windowID, msg.wheel.x, msg.wheel.y, msg.wheel.mouse_x, msg.wheel.mouse_y };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_GAMEPAD_ADDED:
            {
                GamepadAddedEvent event { msg.gdevice.which };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_GAMEPAD_REMOVED:
            {
                GamepadRemovedEvent event { msg.gdevice.which };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            {
                GamepadButtonDownEvent event { msg.gbutton.which, static_cast<eGamepadButton>(msg.gbutton.button) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP:
            {
                GamepadButtonUpEvent event { msg.gbutton.which, static_cast<eGamepadButton>(msg.gbutton.button) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            {
                GamepadAxisMotionEvent event { msg.gaxis.which, static_cast<eGamepadAxis>(msg.gaxis.axis), Max(static_cast<float>(msg.gaxis.value) * kGamepadAxisScale, -1.f) };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DROP_BEGIN:
            {
                DropBeginEvent event { msg.drop.windowID, msg.drop.x, msg.drop.y };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DROP_COMPLETE:
            {
                DropCompleteEvent event { msg.drop.windowID, msg.drop.x, msg.drop.y };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DROP_FILE:
            {
                DropFileEvent event { msg.drop.windowID, msg.drop.x, msg.drop.y, msg.drop.data ? StringView { msg.drop.data } : StringView {} };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_DROP_TEXT:
            {
                DropTextEvent event { msg.drop.windowID, msg.drop.x, msg.drop.y, msg.drop.data ? StringView { msg.drop.data } : StringView {} };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_CLIPBOARD_UPDATE:
            {
                ClipboardUpdatedEvent event {};
                DispatchEvent(event);
            }
            break;

            default:
                break;
        }
    }
}

void Application::DispatchEvent(
    Event& _event)
{
    EventDispatcher dispatcher { _event };
    dispatcher.Dispatch<QuitEvent>(this, &Application::OnQuitEvent_);
    OnEvent(_event);
}

void Application::OnQuitEvent_(
    const QuitEvent&)
{
    Quit();
}

}   // namespace jug
