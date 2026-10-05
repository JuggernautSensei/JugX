#pragma once
#include "AppEvent.h"
#include "Callable.h"
#include "Timer.h"

namespace jug
{

enum class eLoggerOption
{
    None    = 0,        // disable.
    Console = 1 << 0,   // std output
    System  = 1 << 1,   // system output
    Memory  = 1 << 2,   // ring buffer memory logger
    File    = 1 << 3,   // file logger
};

struct AppConfig
{
    StringView appName       = "Application";
    StringView appVersion    = "1.0.0";
    StringView appIdentifier = {};
    bool       bInitGamepad  = false;

    // logging
    Flags<eLoggerOption> loggerFlags   = { eLoggerOption::Console, eLoggerOption::System, eLoggerOption::File };
    size_t               numMemoryLogs = 1024;
    FilePath             logDir        = "Log\\";
};

class App
{
    JUG_CLASS(App, NO_COPY, NO_MOVE)

public:
    [[nodiscard]] static App& GetSingleton();
    virtual ~App();

    int  Run(int _argc, char** _argv);
    void Quit();
    void DispatchEvent(Event& _event);

    void ExecuteOnMainThread(const Callable<void()>& _command);   // thread-safe

    [[nodiscard]] float              GetDeltaTimeSec() const;
    [[nodiscard]] Span<const String> GetCommandLineArgs() const;
    [[nodiscard]] const AppConfig&   GetConfig() const;

protected:
    explicit App(const AppConfig& _config);

    virtual void Init();
    virtual void Shutdown();
    virtual void Update(float _deltaTimeSec);
    virtual void OnEvent(Event& _event);
    virtual void OnSystemEvent(const SDL_Event& _event);

    void SetReturnCode(int _code);

private:
    void ExecuteCommands_();
    void CleanUpCommands_();

    void InitSDL_() const;
    void ShutdownSDL_() const;

    void InitCore_() const;
    void ShutdownCore_();

    void InitLogger_() const;
    void ShutdownLogger_() const;

    void PollSystemEvents_();
    void OnQuitEvent_(const QuitEvent& _event);

    AppConfig m_config = {};

    Timer m_timer        = {};
    float m_deltaTimeSec = 0.f;

    Vector<String> m_cmdArgs    = {};
    bool           m_bRunning   = false;
    int            m_returnCode = 0;

    Vector<Callable<void()>> m_penddingCmds;
    Vector<Callable<void()>> m_executeCmds;
    std::mutex               m_cmdBufMtx = {};

    Vector<Callable<void()>> m_shutdownCmds;
    std::mutex               m_shutdownCmdBufMtx = {};
};

}   // namespace jug
