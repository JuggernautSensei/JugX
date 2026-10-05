#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#    define JUG_DEBUG             1
#    define JUG_BUILD_CONFIG_NAME "Debug"
#else
#    define JUG_RELEASE           1
#    define JUG_BUILD_CONFIG_NAME "Release"
#endif

#ifndef JUG_ENABLE_PROFILE
#    define JUG_ENABLE_PROFILE 1
#endif