#pragma once
#include <cstdint>
#include <type_traits>

#include "Assertion.h"
#include "Config.h"

namespace jug
{

// ================================================================
//  HandleT
//   포인터 대신 쓸 수 있는 32bit 핸들 타입.
//   24bit index + 8bit token으로 구성.
//   struct ResourceTag {};
//   using ResourceHandle = HandleT<ResourceTag>;
//   이런 식으로 사용하면 강타입의 핸들 타입을 만들 수 있음.
// ================================================================

namespace handle_detail
{
    constexpr uint32_t kNullValue = 0xFFFFFFFF;
    constexpr uint32_t kMaxIndex  = 0xFFFFFF - 1;   // 24 bits for index.
    constexpr uint32_t kMaxToken  = 0xFF;           // 8 bits for token.
                                                    // 0xFFFFFFFF 를 null handle sentinel로 사용하기 때문에 MaxIndex 값을 0xFFFFFF - 1로 설정.
}   // namespace handle_detail

struct NullHandleType
{
    struct Tag
    {
    };

    constexpr explicit NullHandleType(Tag)
    {
    }
};

constexpr NullHandleType kNullHandle { NullHandleType::Tag {} };

template<typename T>
class Handle
{
public:
    constexpr Handle()
        : value(handle_detail::kNullValue)
    {
    }

    /* implicit */ constexpr Handle(
        const NullHandleType)
        : value(handle_detail::kNullValue)
    {
    }

    constexpr Handle(
        const uint32_t _index,
        const uint8_t  _token)
        : index(_index)
        , token(_token)
    {
        JUG_ASSERT(_index <= handle_detail::kMaxIndex, "Index out of range. Must be <= kMaxIndex.\n");
    }

    constexpr explicit Handle(
        const uint32_t _value)
        : value(_value)
    {
    }

    constexpr Handle(
        Handle&& _other) noexcept
        : value(std::exchange(_other.value, handle_detail::kNullValue))
    {
    }

    constexpr Handle& operator=(
        const NullHandleType)
    {
        value = handle_detail::kNullValue;
        return *this;
    }

    constexpr Handle& operator=(
        Handle&& _other) noexcept
    {
        if (this != &_other)
        {
            value = std::exchange(_other.value, handle_detail::kNullValue);
        }
        return *this;
    }

    constexpr Handle(const Handle&)            = default;
    constexpr Handle& operator=(const Handle&) = default;
    constexpr ~Handle()                        = default;

    [[nodiscard]] constexpr bool operator==(const Handle& _other) const  = default;
    [[nodiscard]] constexpr auto operator<=>(const Handle& _other) const = default;

    [[nodiscard]] constexpr bool IsNull() const
    {
        return value == handle_detail::kNullValue;
    }

    constexpr explicit operator bool() const
    {
        return !IsNull();
    }

    [[nodiscard]] constexpr uint32_t GetIndex() const
    {
        return index;
    }

    [[nodiscard]] constexpr uint32_t GetToken() const
    {
        return token;
    }

    [[nodiscard]] constexpr uint32_t GetValue() const
    {
        return value;
    }

private:
    union
    {
        struct
        {
            uint32_t index : 24;
            uint32_t token : 8;
        };

        uint32_t value = handle_detail::kNullValue;
    };
};

}   // namespace jug

// std::hash specialization
template<typename T>
struct std::hash<jug::Handle<T>>
{
    size_t operator()(
        const jug::Handle<T>& _handle) const
    {
        return std::hash<uint32_t>()(_handle.GetValue());
    }
};
