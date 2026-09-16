#pragma once

namespace jug
{

namespace type_refl_detail
{
    template<typename T>
    [[nodiscard]] constexpr StringView ExtractFullName()
    {
        constexpr StringView kName = JUG_FUNCSIG;
#if defined(JUG_COMPILER_CLANG) || defined(JUG_COMPILER_GCC)
        constexpr StringView kMarker    = "T = ";
        constexpr size_t     kMarkerPos = kName.find(kMarker);
        static_assert(kMarkerPos != StringView::npos);   // NOLINT
        constexpr size_t kBegin = kMarkerPos + kMarker.size();
        size_t           endPos = StringView::npos;
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
#elif defined(JUG_COMPILER_MSVC)
        constexpr size_t kBegin = kName.rfind('<');
        constexpr size_t kEnd   = kName.rfind('>');
        static_assert(kBegin != StringView::npos && kEnd != StringView::npos);
        return kName.substr(kBegin + 1, kEnd - kBegin - 1);
#else
#    error "TypeRefl: unsupported compiler (requires MSVC, Clang, or GCC)"
#endif
    }

    [[nodiscard]] constexpr StringView ExtractName(
        const StringView _fullName)
    {
        size_t depth = 0;
        size_t colon = StringView::npos;
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
        return colon == StringView::npos ? _fullName : _fullName.substr(colon + 2);
    }

    template<typename T>
    constexpr StringView kFullName = ExtractFullName<T>();

    template<typename T>
    constexpr StringView kName = ExtractName(kFullName<T>);

    template<typename T>
    constexpr uint64_t kHash = HashString(kFullName<T>);
}   // namespace type_refl_detail

// =========================================================
//  Public API
// =========================================================

template<typename T>
[[nodiscard]] constexpr StringView FullNameOf()
{
    return type_refl_detail::kFullName<T>;
}

template<typename T>
[[nodiscard]] constexpr StringView NameOf()
{
    return type_refl_detail::kName<T>;
}

template<typename T>
[[nodiscard]] constexpr uint64_t HashOf()
{
    return type_refl_detail::kHash<T>;
}

}   // namespace jug
