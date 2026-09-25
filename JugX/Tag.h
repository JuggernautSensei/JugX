#pragma once

#define JUG_DEFINE_TAG(_type, _value)          \
    struct _type                               \
    {                                          \
        struct Tag                             \
        {                                      \
        };                                     \
        constexpr explicit _type(const Tag) {} \
    };                                         \
    inline constexpr _type _value { _type::Tag {} };

namespace jug
{

JUG_DEFINE_TAG(NoInitType, kNoInit);

}