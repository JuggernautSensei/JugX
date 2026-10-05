#include "pch.h"

#include "OS.h"

#include "OsInclude.h"
#include "StringEncoder.h"
#include "StringFormat.h"

#ifdef JUG_OS_WINDOWS
#    pragma comment(lib, "advapi32.lib")
#endif

namespace jug
{

void* GetNativeWindowHandle(
    SDL_Window* _pWindow)
{
    JUG_ASSERT(_pWindow, "Invalid window pointer");
    const SDL_PropertiesID props = SDL_GetWindowProperties(_pWindow);
#ifdef JUG_OS_WINDOWS
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#else
#    error "Unsupported platform"
#endif
}

void* GetNativeWindowHandle(
    const SDL_WindowID _id)
{
    JUG_ASSERT(_id, "Invalid window ID");
    return GetNativeWindowHandle(SDL_GetWindowFromID(_id));
}

String GetOsVersionName()
{
#ifdef JUG_OS_WINDOWS
    // GetVersionEx 는 매니페스트가 없으면 8.1로 거짓말한다. ntdll 의 RtlGetVersion 은 실제 값을 준다.
    using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

    const HMODULE hNtdll    = ::GetModuleHandleW(L"ntdll.dll");
    const auto    pFnOrNull = hNtdll ? reinterpret_cast<RtlGetVersionFn>(::GetProcAddress(hNtdll, "RtlGetVersion")) : nullptr;   // NOLINT
    if (!pFnOrNull)
    {
        return "Windows (unknown)";
    }

    OSVERSIONINFOW vi      = {};
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (pFnOrNull(&vi) != 0)
    {
        return "Windows (unknown)";
    }

    // Windows 11 도 major 10 이다. 빌드 22000 부터 11.
    const StringView name = vi.dwMajorVersion == 10 && vi.dwBuildNumber >= 22000 ? "Windows 11" : vi.dwMajorVersion == 10 ? "Windows 10" : "Windows";
    return Format("{} ({}.{}.{})", name, vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber);
#else
    return SDL_GetPlatform();
#endif
}

String GetCpuName()
{
#ifdef JUG_OS_WINDOWS
    // cpuid 는 x86 전용이라 ARM64 까지 되는 레지스트리를 쓴다.
    wchar_t buffer[256] = {};
    DWORD   size        = sizeof(buffer);
    if (::RegGetValueW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", L"ProcessorNameString", RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS)
    {
        return "unknown";
    }

    String name = {};
    AppendUtf16(name, WStringView { buffer });

    // 일부 CPU 는 이름 앞뒤에 공백이 붙어 있다.
    const size_t first = name.find_first_not_of(' ');
    const size_t last  = name.find_last_not_of(' ');
    return first == String::npos ? String { "unknown" } : name.substr(first, last - first + 1);
#else
    return "unknown";
#endif
}

}   // namespace jug