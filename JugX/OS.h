#pragma once
#include <SDL3/SDL.h>

namespace jug
{

[[nodiscard]] void* GetNativeWindowHandle(SDL_Window* _pWindow);
[[nodiscard]] void* GetNativeWindowHandle(SDL_WindowID _id);

}   // namespace jug
