// EnumRefl 동작 검증용 테스트 (컴파일 타임 static_assert 위주)
//
// 탐색 범위는 전역 매크로 JUG_ENUM_REFL_RANGE_MIN / JUG_ENUM_REFL_RANGE_MAX (기본 [0, 127]) 하나뿐이다.
// 범위 밖 enumerator 를 가진 enum 은 리플렉션할 수 없다.
//
// RangeOf() / NamesOf() / GetIndex()의 순서는 값 오름차순이다.
#include "pch.h"

namespace enum_refl_test
{

enum class eDefault
{
    A = 0,
    B = 1,
    C = 5,
};

enum class eDenseDefault
{
    X,
    Y,
    Z,
};

enum class eEdge : int
{
    First = 0,
    Last  = 127,
};

enum class eUnordered : int
{
    Third  = 3,
    First  = 1,
    Second = 2,
};

enum class eUnderlyingU8 : uint8_t
{
    Lo = 10,
    Hi = 100,
};

}   // namespace enum_refl_test

namespace
{

using enum_refl_test::eDefault;
using enum_refl_test::eDenseDefault;
using enum_refl_test::eEdge;
using enum_refl_test::eUnordered;
using enum_refl_test::eUnderlyingU8;

// ==========================================================
//  전역 범위
// ==========================================================

static_assert(enum_relf_detail::kReflRangeMin == JUG_ENUM_REFL_RANGE_MIN);
static_assert(enum_relf_detail::kReflRangeMax == JUG_ENUM_REFL_RANGE_MAX);
static_assert(enum_relf_detail::kReflRange == JUG_ENUM_REFL_RANGE_MAX - JUG_ENUM_REFL_RANGE_MIN + 1);

// ==========================================================
//  1. 기본 범위
// ==========================================================

static_assert(CountOf<eDefault>() == 3);
static_assert(IsValid(eDefault::A) && IsValid(eDefault::B) && IsValid(eDefault::C));
static_assert(!IsValid(static_cast<eDefault>(2)));
static_assert(!IsValid(static_cast<eDefault>(-1)));
static_assert(!IsValid(static_cast<eDefault>(6)));
static_assert(!IsValid(static_cast<eDefault>(128)));

static_assert(NameOf(eDefault::C) == "C");
static_assert(FullNameOf(eDefault::C).ends_with("eDefault::C"));
static_assert(ParseAs<eDefault>("B").HasValue() && *ParseAs<eDefault>("B") == eDefault::B);
static_assert(!ParseAs<eDefault>("D").HasValue());
static_assert(ParseAs<eDefault>(5).HasValue() && *ParseAs<eDefault>(5) == eDefault::C);
static_assert(!ParseAs<eDefault>(2).HasValue());

static_assert(Min<eDefault>() == eDefault::A);
static_assert(Max<eDefault>() == eDefault::C);
static_assert(RangeOf<eDefault>().size() == 3);
static_assert(RangeOf<eDefault>()[0] == eDefault::A && RangeOf<eDefault>()[1] == eDefault::B && RangeOf<eDefault>()[2] == eDefault::C);
static_assert(GetIndex(eDefault::C) == 2);

static_assert(!ContiguousEnumT<eDefault>);
static_assert(ContiguousEnumT<eDenseDefault> && DirectIndexableEnumT<eDenseDefault>);

// ==========================================================
//  2. 범위 경계
// ==========================================================

static_assert(CountOf<eEdge>() == 2);
static_assert(IsValid(eEdge::First) && IsValid(eEdge::Last));
static_assert(!IsValid(static_cast<eEdge>(-1)));
static_assert(!IsValid(static_cast<eEdge>(1)));
static_assert(!IsValid(static_cast<eEdge>(126)));
static_assert(!IsValid(static_cast<eEdge>(128)));
static_assert(!IsValid(static_cast<eEdge>(100000)));
static_assert(Min<eEdge>() == eEdge::First && Max<eEdge>() == eEdge::Last);
static_assert(NameOf(eEdge::Last) == "Last");
static_assert(ParseAs<eEdge>(127).HasValue() && *ParseAs<eEdge>(127) == eEdge::Last);
static_assert(!ParseAs<eEdge>(128).HasValue());
static_assert(GetIndex(eEdge::First) == 0 && GetIndex(eEdge::Last) == 1);

// ==========================================================
//  3. 선언 순서와 무관하게 값 오름차순
// ==========================================================

static_assert(CountOf<eUnordered>() == 3);
static_assert(Min<eUnordered>() == eUnordered::First && Max<eUnordered>() == eUnordered::Third);
static_assert(RangeOf<eUnordered>()[0] == eUnordered::First && RangeOf<eUnordered>()[1] == eUnordered::Second && RangeOf<eUnordered>()[2] == eUnordered::Third);
static_assert(NamesOf<eUnordered>()[0] == "First" && NamesOf<eUnordered>()[1] == "Second" && NamesOf<eUnordered>()[2] == "Third");
static_assert(GetIndex(eUnordered::First) == 0 && GetIndex(eUnordered::Second) == 1 && GetIndex(eUnordered::Third) == 2);
static_assert(ContiguousEnumT<eUnordered> && !DirectIndexableEnumT<eUnordered>);

// ==========================================================
//  4. underlying type 이 작은 enum
// ==========================================================

static_assert(CountOf<eUnderlyingU8>() == 2);
static_assert(IsValid(eUnderlyingU8::Lo) && IsValid(eUnderlyingU8::Hi));
static_assert(!IsValid(static_cast<eUnderlyingU8>(0)));
static_assert(!IsValid(static_cast<eUnderlyingU8>(255)));
static_assert(NameOf(eUnderlyingU8::Hi) == "Hi");
static_assert(ParseAs<eUnderlyingU8>(static_cast<uint8_t>(10)).HasValue());
static_assert(!ParseAs<eUnderlyingU8>(static_cast<uint8_t>(11)).HasValue());
static_assert(!ContiguousEnumT<eUnderlyingU8>);

}   // namespace

TEST(EnumRefl, CompileTimeChecksCompiled)
{
    EXPECT_EQ(CountOf<eDefault>(), 3u);
    EXPECT_EQ(CountOf<eUnordered>(), 3u);
}
