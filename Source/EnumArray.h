#pragma once
#include <algorithm>

#include "EnumRefl.h"

#define DT enum_refl_detail

namespace jug
{

// ===========================================================================
//  ENUM_ARRAY
//   zero overhead를 목표로한 c-style 배열 like container
//   키를 Enum의 값으로하여 간편하게 접근할 수 있음
// ===========================================================================

template<EnumReflT TEnum, typename TValue>
    requires std::is_object_v<TValue>
struct ENUM_ARRAY
{
    using Iterator      = TValue*;
    using ConstIterator = const TValue*;

    constexpr static size_t kSize = CountOf<TEnum>();

    // =========================================================
    //  Access & Utils
    // =========================================================

    [[nodiscard]] constexpr TValue& operator[](
        const TEnum _i)
    {
        if constexpr (DirectIndexableEnumT<TEnum>)
        {
            return elems[static_cast<size_t>(_i)];   // fast access for contiguous enums starting at 0
        }
        else
        {
            const size_t index = GetSequence(_i);
            return elems[index];
        }
    }

    [[nodiscard]] constexpr const TValue& operator[](
        const TEnum _e) const
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

    TValue elems[kSize];   // NOLINT
};

// ==========================================================
//  Direct Enum Array
//   Direct Indexing이 가능한 Enum만 key로 쓸 수 있는 배열
//   대부분의 경우 ENUM_ARRAY가 상위 개념이며 이것만 사용해도 됨.
//   CUDA와 범용가능한 코드를 만들어야 할 경우 EnumRelf을 사용하지 못하기 때문에
//   ENUM_ARRAY를 사용하고 싶다면 반드시 DirectIndexable해야함
//   그것을 명시적으로 표현하기위한 클래스.
//   실제로는 using 및 requires로 충분하지만.
//   Visual Studio 파서가 컴파일 타임 리플렉션을 파싱시에 제대로 처리하지 못해
//   오류가 아님에도 오류로 인식해버림. 그래서 static_assert로 처리함.
// ==========================================================

// 원래는 아래 코드가 맞으나, 오류가 간주된다. (빌드는 됨)
// template<EnumReflT TEnum, typename TValue>
//     requires DirectIndexableEnumT<TEnum>
// using DIRECT_ENUM_ARRAY = ENUM_ARRAY<TEnum, TValue>;

template<EnumReflT TEnum, typename TValue>
struct DIRECT_ENUM_ARRAY : public ENUM_ARRAY<TEnum, TValue>
{
    static_assert(DirectIndexableEnumT<TEnum>, "DIRECT_ENUM_ARRAY: TEnum must be direct indexable");   // intellisense가 requires를 잘못 인식하는 경우가 있어 static_assert를 사용함.

    using SuperT = ENUM_ARRAY<TEnum, TValue>;
    using SuperT::GetPtr;
    using SuperT::GetSize;

    DIRECT_ENUM_ARRAY() = default;

    /* implicit */ DIRECT_ENUM_ARRAY(
        const SuperT& _other)
    {
        std::copy_n(_other.GetPtr(), _other.GetSize(), GetPtr());
    }

    /* implicit */ DIRECT_ENUM_ARRAY(
        SuperT&& _other)
    {
        std::move(_other.GetPtr(), _other.GetPtr() + _other.GetSize(), GetPtr());
    }

    DIRECT_ENUM_ARRAY& operator=(
        const SuperT& _other)
    {
        std::copy_n(_other.GetPtr(), _other.GetSize(), GetPtr());
        return *this;
    }

    DIRECT_ENUM_ARRAY& operator=(
        SuperT&& _other)
    {
        std::move(_other.GetPtr(), _other.GetPtr() + _other.GetSize(), GetPtr());
        return *this;
    }
};

}   // namespace jug

#undef DT