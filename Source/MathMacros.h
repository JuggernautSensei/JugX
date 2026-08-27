#pragma once

#ifdef __CUDACC__
#    define JUG_MATH_API __host__ __device__
#else
#    define JUG_MATH_API
#endif