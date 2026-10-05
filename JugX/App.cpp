#include "pch.h"
#include "App.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#if JUG_OS_WINDOWS
#    include <spdlog/sinks/msvc_sink.h>
#else
#    include <spdlog/sinks/syslog_sink.h>
#endif

#include "CoreLogger.h"
#include "EventDispatcher.h"
#include "Fatal.h"
#include "OS.h"
#include "StringFormat.h"
#include "Timer.h"
#include "TimeStamp.h"

namespace jug
{

namespace
{
    App* g_pSingleton = nullptr;

    constexpr float kGamepadAxisScale = 1.f / 32767.f;

    [[nodiscard]] Flags<eKeyMod> MakeKeyMods_(
        const SDL_Keymod _mod)
    {
        Flags<eKeyMod> flags = kNoneFlags;
        if (_mod & SDL_KMOD_SHIFT)
        {
            flags |= eKeyMod::Shift;
        }
        if (_mod & SDL_KMOD_CTRL)
        {
            flags |= eKeyMod::Ctrl;
        }
        if (_mod & SDL_KMOD_ALT)
        {
            flags |= eKeyMod::Alt;
        }
        if (_mod & SDL_KMOD_GUI)
        {
            flags |= eKeyMod::Gui;
        }
        if (_mod & SDL_KMOD_NUM)
        {
            flags |= eKeyMod::Num;
        }
        if (_mod & SDL_KMOD_CAPS)
        {
            flags |= eKeyMod::Caps;
        }
        if (_mod & SDL_KMOD_SCROLL)
        {
            flags |= eKeyMod::Scroll;
        }
        if (_mod & SDL_KMOD_MODE)
        {
            flags |= eKeyMod::Mode;
        }
        return flags;
    }

    void LogSystemInfo_(
        const AppConfig&         _config,
        const Span<const String> _cmdArgs)
    {
        // app
        JUG_CORE_LOG_INFO("{} {}{}{}{}", _config.appName, _config.appVersion, _config.appIdentifier.empty() ? "" : " (", _config.appIdentifier, _config.appIdentifier.empty() ? "" : ")");

        // arch / platform
        JUG_CORE_LOG_INFO("    Build  : {} {}, {} {}.{}.{}, C++{}", JUG_BUILD_CONFIG_NAME, JUG_ARCH_NAME, JUG_COMPILER_NAME, JUG_COMPILER_VERSION_MAJOR, JUG_COMPILER_VERSION_MINOR, JUG_COMPILER_VERSION_PATCH, JUG_CPP_VERSION);
        JUG_CORE_LOG_INFO("    OS     : {}", GetOsVersionName());
        JUG_CORE_LOG_INFO("    CPU    : {}, {} logical cores, SIMD:{} {} {} {}", GetCpuName(), SDL_GetNumLogicalCPUCores(), SDL_HasSSE42() ? "SSE4.2" : "", SDL_HasAVX2() ? "AVX2" : "", SDL_HasAVX512F() ? "AVX-512F" : "", SDL_HasNEON() ? "NEON" : "");
        JUG_CORE_LOG_INFO("    Memory : {} MiB", SDL_GetSystemRAM());

        // SDL
        const int sdlVersion = SDL_GetVersion();
        JUG_CORE_LOG_INFO("    SDL    : {}.{}.{} ({})", SDL_VERSIONNUM_MAJOR(sdlVersion), SDL_VERSIONNUM_MINOR(sdlVersion), SDL_VERSIONNUM_MICRO(sdlVersion), SDL_GetRevision());

        // 실행 파일 경로
        std::error_code ec  = {};
        const FilePath  cwd = fs::current_path(ec);
        if (ec)
        {
            JUG_CORE_LOG_INFO("    Path   : exe '{}', cwd '?' (error: {})", SDL_GetBasePath() ? SDL_GetBasePath() : "?", ec.message());
        }
        else
        {
            JUG_CORE_LOG_INFO("    Path   : exe '{}', cwd '{}'", SDL_GetBasePath() ? SDL_GetBasePath() : "?", cwd.string());
        }

        // command line args
        String args = {};
        for (size_t i = 1; i < _cmdArgs.size(); ++i)
        {
            AppendFormat(args, "  {}", _cmdArgs[i]);
        }
        JUG_CORE_LOG_INFO("    Args   :{}", args.empty() ? " (none)" : args);
    }
}   // namespace

App::App(
    const AppConfig& _config)
    : m_config(_config)
{
    JUG_ASSERT(!g_pSingleton, "App instance already exists.");
    g_pSingleton = this;
}

void App::Init()
{
}

void App::Shutdown()
{
}

void App::OnEvent(Event& _event)
{
}

void App::Update(
    const float _deltaTimeSec)
{
}

void App::OnSystemEvent(
    const SDL_Event& _event)
{
}

App::~App()
{
    JUG_ASSERT(g_pSingleton == this, "App instance mismatch.");
    g_pSingleton = nullptr;
}

App& App::GetSingleton()
{
    JUG_ASSERT(g_pSingleton, "App instance is not created yet.");
    return *g_pSingleton;
}

int App::Run(
    const int _argc,
    char**    _argv)
{
    m_cmdArgs.reserve(static_cast<size_t>(_argc));
    for (int i = 0; i < _argc; ++i)
    {
        m_cmdArgs.emplace_back(_argv[i]);
    }

    // init
    m_timer.Start();
    InitCore_();
    LogSystemInfo_(m_config, m_cmdArgs);
    Init();
    m_timer.Stop();
    JUG_CORE_LOG_INFO("App initialized in {:.1f} ms", TimeCastF(m_timer.GetTotalCount(), eTimeUnit::Milli));

    m_bRunning   = true;
    m_returnCode = 0;

    // start loop
    m_timer.Reset();
    m_timer.Start();
    int numFrames = 0;
    while (m_bRunning)
    {
        m_timer.Lap();
        m_deltaTimeSec = TimeCastF(m_timer.GetElapsedCount(), eTimeUnit::Sec);

        PollSystemEvents_();
        ExecuteCommands_();
        if (!m_bRunning)   // command와 event handling 중 quit 호출
        {
            break;
        }

        Update(m_deltaTimeSec);
        ++numFrames;
    }

    // shutdown log
    const float ranTimeSec = TimeCastF(m_timer.GetTotalCount(), eTimeUnit::Sec);
    JUG_CORE_LOG_INFO("App shutting down. exit code = {}, ran {:.1f} s, {} frames (avg {:.2f} ms/frame)", m_returnCode, ranTimeSec, numFrames, numFrames > 0 ? ranTimeSec * 1000.0 / static_cast<double>(numFrames) : 0.0);

    // shutdown
    Shutdown();
    ShutdownCore_();
    return m_returnCode;
}

void App::Quit()
{
    m_bRunning = false;
}

// ===========================================
//  Time
// ===========================================

void App::DispatchEvent(
    Event& _event)
{
    EventDispatcher dispatcher { _event };
    dispatcher.Dispatch<QuitEvent>(this, &App::OnQuitEvent_);
    OnEvent(_event);
}

void App::ExecuteOnMainThread(
    const Callable<void()>& _command)
{
    std::scoped_lock lock { m_cmdBufMtx };
    m_penddingCmds.emplace_back(_command);
}

float App::GetDeltaTimeSec() const
{
    return m_deltaTimeSec;
}

Span<const String> App::GetCommandLineArgs() const
{
    return m_cmdArgs;
}

const AppConfig& App::GetConfig() const
{
    return m_config;
}

void App::SetReturnCode(
    const int _code)
{
    m_returnCode = _code;
}

void App::ExecuteCommands_()
{
    {
        std::scoped_lock lock { m_cmdBufMtx };
        std::swap(m_penddingCmds, m_executeCmds);
    }

    for (Callable<void()>& cmd: m_executeCmds)
    {
        cmd();
    }
    m_executeCmds.clear();
}

void App::CleanUpCommands_()
{
    {
        std::scoped_lock lock { m_cmdBufMtx };
        std::swap(m_penddingCmds, m_executeCmds);
    }

    if (!m_executeCmds.empty())
    {
        JUG_CORE_LOG_ERROR("{} commands are not executed before shutdown.", m_executeCmds.size());
    }
    m_executeCmds.clear();
}

void App::InitSDL_() const
{
    const String appName    = String { m_config.appName };
    const String appVersion = String { m_config.appVersion };
    const String appIdent   = String { m_config.appIdentifier };
    if (!SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appIdent.empty() ? nullptr : appIdent.c_str()))
    {
        JUG_CORE_LOG_WARN("SDL_SetAppMetadata failed: {}", SDL_GetError());
    }

    SDL_InitFlags flags = SDL_INIT_VIDEO;
    if (m_config.bInitGamepad)
    {
        flags |= SDL_INIT_GAMEPAD;
    }

    if (!SDL_Init(flags))
    {
        JUG_FATAL("SDL_Init failed: {}", SDL_GetError());
    }
}

void App::ShutdownSDL_() const
{
    SDL_Quit();
}

void App::InitCore_() const
{
    InitSDL_();
}

void App::ShutdownCore_()
{
    CleanUpCommands_();
    ShutdownSDL_();
}

void App::InitLogger_() const
{
    if (m_config.loggerFlags == eLoggerOption::None)
    {
        return;
    }

    ARRAY<spdlog::sink_ptr, 16> sinks    = {};
    size_t                      numSinks = 0;

    if (m_config.loggerFlags & eLoggerOption::Console)
    {
        sinks[numSinks++] = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }

    if (m_config.loggerFlags & eLoggerOption::System)
    {
#if JUG_OS_WINDOWS
        sinks[numSinks++] = std::make_shared<spdlog::sinks::windebug_sink_mt>();
#else
        sinks[numSinks++] = std::make_shared<spdlog::sinks::syslog_sink_mt>(String { m_config.appName }, 0, LOG_USER, true);
#endif
    }

    if (m_config.loggerFlags & eLoggerOption::Memory)
    {
        sinks[numSinks++] = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(m_config.numMemoryLogs);
    }

    if (m_config.loggerFlags & eLoggerOption::File)
    {
        const TimeStamp ts  = TimeStamp::Now();
        FilePath        dir = m_config.logDir.empty() ? "Log" : m_config.logDir;
        dir /= Format("{}y{}m{}d {}h{}m{}s", ts.year, ts.month, ts.dayOfTheMonth, ts.hour, ts.min, ts.sec);
        sinks[numSinks++] = std::make_shared<spdlog::sinks::basic_file_sink_mt>(dir.string(), true);
    }

    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("[Core]", sinks.begin(), sinks.begin() + numSinks);
}

void App::ShutdownLogger_() const
{
}

void App::PollSystemEvents_()
{
    SDL_Event msg;
    while (SDL_PollEvent(&msg))
    {
        OnSystemEvent(msg);

        switch (msg.type)
        {
            case SDL_EVENT_QUIT:
            {
                QuitEvent event = {};
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_LOW_MEMORY:
            {
                LowMemoryEvent event = {};
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
                const SDL_Window*              pWindowOrNull = SDL_GetWindowFromID(msg.window.windowID);
                const float                    scale         = pWindowOrNull ? SDL_GetWindowDisplayScale(SDL_GetWindowFromID(msg.window.windowID)) : 1.f;
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
                if (msg.key.scancode == SDL_SCANCODE_UNKNOWN)
                {
                    break;
                }

                KeyDownEvent event { msg.key.windowID, static_cast<eKey>(msg.key.scancode), MakeKeyMods_(msg.key.mod), msg.key.repeat };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_KEY_UP:
            {
                if (msg.key.scancode == SDL_SCANCODE_UNKNOWN)
                {
                    break;
                }

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
                if (!IsValid(static_cast<eMouse>(msg.button.button)))
                {
                    break;
                }

                MouseButtonDownEvent event { msg.button.windowID, static_cast<eMouse>(msg.button.button), msg.button.x, msg.button.y, msg.button.clicks };
                DispatchEvent(event);
            }
            break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (!IsValid(static_cast<eMouse>(msg.button.button)))
                {
                    break;
                }

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

void App::OnQuitEvent_(
    const QuitEvent&)
{
    Quit();
}

}   // namespace jug
