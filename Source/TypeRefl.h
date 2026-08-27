#pragma once
#include <string_view>

#include "Macros.h"
#include "StringHasher.h"
#define DT type_refl_detail

namespace jug
{
namespace DT
{
    template<typename T>
    [[nodiscard]] constexpr std::string_view ExtractFullName()
    {
        constexpr std::string_view kName = JUG_PRETTY_FUNCTION;
#if defined(__clang__) || defined(__GNUC__)
        constexpr std::string_view kMarker    = "T = ";
        constexpr size_t           kMarkerPos = kName.find(kMarker);
        static_assert(kMarkerPos != std::string_view::npos);   // NOLINT
        constexpr size_t kBegin = kMarkerPos + kMarker.size();
        size_t           endPos = std::string_view::npos;
        size_t           depth  = 0;
        for (size_t i = kBegin; i < kName.size(); ++i)
        {
            const char c = kName[i];
            if (c == '<' || c == '(')
            {
                ++depth;
            }
            else if (c == '>' || c == ')')
            {
                if (depth > 0) --depth;
            }
            else if (depth == 0 && (c == ';' || c == ',' || c == ']'))
            {
                endPos = i;
                break;
            }
        }
        return kName.substr(kBegin, endPos - kBegin);
#elif defined(_MSC_VER)
        constexpr size_t kBegin = kName.rfind('<');
        constexpr size_t kEnd   = kName.rfind('>');
        static_assert(kBegin != std::string_view::npos && kEnd != std::string_view::npos);
        return kName.substr(kBegin + 1, kEnd - kBegin - 1);
#else
#    error "TypeRefl: unsupported compiler (requires MSVC, Clang, or GCC)"
#endif
    }

    [[nodiscard]] constexpr std::string_view ExtractName(
        const std::string_view _fullName)
    {
        size_t depth = 0;
        size_t colon = std::string_view::npos;
        for (size_t i = _fullName.size(); i-- > 0;)
        {
            const char c = _fullName[i];
            if (c == '>' || c == ')')
            {
                ++depth;
            }
            else if (c == '<' || c == '(')
            {
                if (depth > 0) --depth;
            }
            else if (depth == 0 && c == ':' && i > 0 && _fullName[i - 1] == ':')
            {
                colon = i - 1;
                break;
            }
        }
        return colon == std::string_view::npos ? _fullName : _fullName.substr(colon + 2);
    }

    [[nodiscard]] constexpr uint64_t HashOf(
        const std::string_view _fullName)
    {
        Fnv1a64 hasher;
        hasher.Mix(_fullName);
        return hasher.GetHash();
    }

    template<typename T>
    constexpr std::string_view kFullName = ExtractFullName<T>();

    template<typename T>
    constexpr std::string_view kName = ExtractName(kFullName<T>);

    template<typename T>
    constexpr uint64_t kHash = HashOf(kFullName<T>);
}   // namespace DT

// =========================================================
//  Public API
// =========================================================

template<typename T>
[[nodiscard]] constexpr std::string_view FullNameOf()
{
    return DT::kFullName<T>;
}

template<typename T>
[[nodiscard]] constexpr std::string_view NameOf()
{
    return DT::kName<T>;
}

template<typename T>
[[nodiscard]] constexpr uint64_t HashOf()
{
    return DT::kHash<T>;
}

}   // namespace jug
#undef DT