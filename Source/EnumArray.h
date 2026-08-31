#pragma once
#include <algorithm>

#include "EnumRefl.h"

namespace jug
{

// ===========================================================================
//  ENUM_ARRAY
//   zero overhead를 목표로한 c-style 배열 like container
//   키를 Enum의 값으로하여 간편하게 접근할 수 있음
// ===========================================================================

template<EnumT E, typename TValue>
    requires std::is_object_v<TValue>
struct ENUM_ARRAY
{
    using Iterator      = TValue*;
    using ConstIterator = const TValue*;

    [[nodiscard]] constexpr TValue& operator[](
        const E _i)
    {
        if constexpr (DirectIndexableEnumT<E>)
        {
            return elems[static_cast<size_t>(_i)];   // fast access for contiguous enums starting at 0
        }
        else
        {
            const size_t index = enum_relf_detail::GetIndexOrInvalid<E>(_i);
            return elems[index];
        }
    }

    [[nodiscard]] constexpr const TValue& operator[](
        const E _e) const
    {
        return const_cast<ENUM_ARRAY*>(this)->operator[](_e);
    }

    [[nodiscard]] constexpr TValue& operator[](
        const size_t _i)
    {

        return elems[_i];
    }

    [[nodiscard]] constexpr const TValue& operator[](
        const size_t _i) const
    {
        return const_cast<ENUM_ARRAY*>(this)->operator[](_i);
    }

    constexpr void Fill(
        const TValue& _value)
    {
        std::fill_n(elems, kSize, _value);
    }

    constexpr void Swap(
        ENUM_ARRAY& _other)
    {
        std::swap_ranges(elems, elems + kSize, _other.elems);
    }

    [[nodiscard]] constexpr size_t GetSize() const
    {
        return kSize;
    }

    [[nodiscard]] constexpr TValue* GetPtr()
    {
        return elems;
    }

    [[nodiscard]] constexpr const TValue* GetPtr() const
    {
        return elems;
    }

    [[nodiscard]] constexpr TValue& Front()
    {
        return elems[0];
    }

    [[nodiscard]] constexpr const TValue& Front() const
    {
        return elems[0];
    }

    [[nodiscard]] constexpr TValue& Back()
    {
        return elems[kSize - 1];
    }

    [[nodiscard]] constexpr const TValue& Back() const
    {
        return elems[kSize - 1];
    }

    [[nodiscard]] constexpr Iterator Begin()
    {
        return elems;
    }

    [[nodiscard]] constexpr Iterator End()
    {
        return elems + kSize;
    }

    [[nodiscard]] constexpr ConstIterator Begin() const
    {
        return elems;
    }

    [[nodiscard]] constexpr ConstIterator End() const
    {
        return elems + kSize;
    }

    [[nodiscard]] constexpr ConstIterator CBegin() const
    {
        return elems;
    }

    [[nodiscard]] constexpr ConstIterator CEnd() const
    {
        return elems + kSize;
    }

    // ==========================================================
    //  STL like
    // ==========================================================

    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] constexpr size_t size() const
    {
        return kSize;
    }

    [[nodiscard]] constexpr TValue* data()
    {
        return elems;
    }

    [[nodiscard]] constexpr const TValue* data() const
    {
        return elems;
    }

    [[nodiscard]] constexpr iterator begin()
    {
        return Begin();
    }

    [[nodiscard]] constexpr iterator end()
    {
        return End();
    }

    [[nodiscard]] constexpr const_iterator begin() const
    {
        return Begin();
    }

    [[nodiscard]] constexpr const_iterator end() const
    {
        return End();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const
    {
        return CBegin();
    }

    [[nodiscard]] constexpr const_iterator cend() const
    {
        return CEnd();
    }

    // ==========================================================
    //  Fields
    // ==========================================================

    constexpr static size_t kSize = CountOf<E>();

    TValue elems[kSize];   // NOLINT
};

// =================================================================
//  Direct Enum Array
//   Direct Indexing이 가능한 Enum만 key로 쓸 수 있는 배열
//   CUDA와 범용가능한 코드를 만들어야 할 경우 리플렉션 사용하지 못하기 때문에
//   이런 시나리오에서 ENUM_ARRAY를 사용하고 싶다면, 반드시 DirectIndexable해야함
// =================================================================

template<EnumT E, typename TValue>
struct DIRECT_ENUM_ARRAY : public ENUM_ARRAY<E, TValue>
{
    static_assert(DirectIndexableEnumT<E>, "DIRECT_ENUM_ARRAY: E must be direct indexable");   // intellisense가 requires를 잘못 인식하는 경우가 있어 static_assert를 사용함.
};

}   // namespace jug
