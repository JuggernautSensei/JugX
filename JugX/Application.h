#pragma once
#include "ApplicationEvent.h"
#include "Timer.h"

namespace jug
{

struct ApplicationDesc
{
    StringView appName       = "Application";
    StringView appVersion    = "1.0.0";
    StringView appIdentifier = {};
    bool       bInitGamepad  = true;
};

class Application
{
    JUG_CLASS(Application, NO_COPY, NO_MOVE)

public:
    virtual ~Application();
    [[nodiscard]] static Application& GetSingleton();

    int  Run(int _argc, char** _argv);
    void Quit();
    void DispatchEvent(Event& _event);

    // ===========================================
    //  Time
    // ===========================================

    [[nodiscard]] float GetDeltaTimeSec() const;

    [[nodiscard]] Span<const String>     GetCommandLineArgs() const;
    [[nodiscard]] const ApplicationDesc& GetDesc() const;

protected:
    explicit Application(const ApplicationDesc& _desc);

    virtual void Init();
    virtual void Shutdown();
    virtual void OnEvent(Event& _event);
    virtual void Update(float _deltaTimeSec);

    void SetReturnCode(int _code);

private:
    void InitSystems_() const;
    void ShutdownSystems_() const;
    void PollEvents_();

    void OnQuitEvent_(const QuitEvent& _event);

    ApplicationDesc m_desc = {};

    // ===========================================
    //  Frame
    // ===========================================

    Timer m_timer        = {};
    float m_deltaTimeSec = 0.f;

    Vector<String> m_cmdArgs    = {};
    bool           m_bRunning   = false;
    int            m_returnCode = 0;
};

}   // namespace jug
