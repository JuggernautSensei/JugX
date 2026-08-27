#pragma once

#ifndef JUG_ASSERT
#    include <cassert>
#    define JUG_ASSERT(_x, _msg) assert((_x) && (_msg))
#endif

