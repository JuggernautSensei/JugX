#pragma once
#include <SDL3/SDL.h>

#include "Typedef.h"

namespace jug
{

[[nodiscard]] void* GetNativeWindowHandle(SDL_Window* _pWindow);
[[nodiscard]] void* GetNativeWindowHandle(SDL_WindowID _id);

// 로그/크래시 리포트용 시스템 정보. 알아낼 수 없으면 "unknown".
[[nodiscard]] String GetOsVersionName();   // "Windows 11 (10.0.26200)"
[[nodiscard]] String GetCpuName();         // "AMD Ryzen 7 7735HS with Radeon Graphics"

}   // namespace jug
