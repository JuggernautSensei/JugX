#include "pch.h"

#include "OS.h"

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

}   // namespace jug