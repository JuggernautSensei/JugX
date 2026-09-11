// RingBuffer 동작 검증용 테스트 (Google Test)
//
// 고정 용량 원형 버퍼. 내부적으로 슬롯 하나를 항상 비워 두어 head==tail로
// 가득 참/빔을 구분하므로, capacity를 N으로 잡으면 실제 최대 보관 개수는 N-1이다.
// 가득 찬 상태에서 Push/Emplace 하면 가장 오래된(head) 원소가 밀려난다.
// 순회 순서(Begin~End, begin~end)는 항상 head -> tail(오래된 것 -> 최신) 순서다.
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../Source/RingBuffer.h"

using namespace jug;

namespace
{
using IntBuffer = RingBuffer<int>;

std::vector<int> ToVector(const IntBuffer& _buffer)
{
    std::vector<int> result;
    for (const int value : _buffer)
    {
        result.push_back(value);
    }
    return result;
}
}   // namespace

// ==========================================================
//  기본 상태
// ==========================================================

TEST(RingBuffer, DefaultCapacityIsEight)
{
    IntBuffer buffer;
    EXPECT_EQ(buffer.GetCapacity(), 8u);
    EXPECT_EQ(buffer.GetMaxSize(), 7u);
    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetSize(), 0u);
}

TEST(RingBuffer, ExplicitCapacityIsRespected)
{
    IntBuffer buffer(4);
    EXPECT_EQ(buffer.GetCapacity(), 4u);
    EXPECT_EQ(buffer.GetMaxSize(), 3u);
}

// ==========================================================
//  Push / Emplace / Pop
// ==========================================================

TEST(RingBuffer, PushIncreasesSizeAndPreservesOrder)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    EXPECT_EQ(buffer.GetSize(), 3u);
    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 1, 2, 3 }));
}

TEST(RingBuffer, EmplaceConstructsInPlace)
{
    RingBuffer<std::string> buffer(4);
    buffer.Emplace(3, 'x');   // std::string(3, 'x')

    EXPECT_EQ(buffer.GetSize(), 1u);
    EXPECT_EQ(buffer.Front(), "xxx");
}

TEST(RingBuffer, PopRemovesFrontElement)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Pop();

    EXPECT_EQ(buffer.GetSize(), 1u);
    EXPECT_EQ(buffer.Front(), 2);
}

TEST(RingBuffer, FirstPushLazilyAllocatesStorage)
{
    // 생성 직후엔 내부 버퍼가 아직 할당되지 않은 상태여야 하고,
    // 첫 Push에서 정상적으로 할당되어 동작해야 한다(할당 누락 회귀 방지).
    IntBuffer buffer(4);
    buffer.Push(42);

    EXPECT_EQ(buffer.GetSize(), 1u);
    EXPECT_EQ(buffer.Front(), 42);
}

// ==========================================================
//  용량 초과 시 오버라이트(축출)
// ==========================================================

TEST(RingBuffer, PushEvictsOldestWhenFull)
{
    IntBuffer buffer(4);   // 최대 3개 보관
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);
    buffer.Push(4);   // 1이 밀려나야 한다.

    EXPECT_EQ(buffer.GetSize(), 3u);
    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 2, 3, 4 }));
}

TEST(RingBuffer, RepeatedOverwriteWrapsAroundCorrectly)
{
    IntBuffer buffer(4);
    for (int i = 0; i < 10; ++i)
    {
        buffer.Push(i);
    }

    // 마지막 3개(7, 8, 9)만 남아야 한다.
    EXPECT_EQ(buffer.GetSize(), 3u);
    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 7, 8, 9 }));
}

// ==========================================================
//  Front / Back / operator[] / At
// ==========================================================

TEST(RingBuffer, FrontAndBackReturnEnds)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    EXPECT_EQ(buffer.Front(), 1);
    EXPECT_EQ(buffer.Back(), 3);
}

TEST(RingBuffer, IndexOperatorAccessesLogicalPosition)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[1], 2);
    EXPECT_EQ(buffer[2], 3);
}

TEST(RingBuffer, IndexOperatorWorksAfterWrapAround)
{
    IntBuffer buffer(4);   // 최대 3개
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);
    buffer.Push(4);   // head가 내부 버퍼 뒷부분으로 이동(wrap) 하는 상태를 만듦

    EXPECT_EQ(buffer[0], 2);
    EXPECT_EQ(buffer[1], 3);
    EXPECT_EQ(buffer[2], 4);
}

TEST(RingBuffer, AtReturnsSameValueAsIndexOperator)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);

    EXPECT_EQ(buffer.At(0), buffer[0]);
    EXPECT_EQ(buffer.At(1), buffer[1]);
}

// ==========================================================
//  Clear / SetCapacity
// ==========================================================

TEST(RingBuffer, ClearRemovesAllElements)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Clear();

    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_EQ(buffer.GetSize(), 0u);
}

TEST(RingBuffer, SetCapacityGrowsAndPreservesOrder)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    buffer.SetCapacity(8);
    EXPECT_EQ(buffer.GetCapacity(), 8u);
    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 1, 2, 3 }));
}

TEST(RingBuffer, SetCapacityShrinksAndKeepsOldestElements)
{
    // 축소 시 head(가장 오래된 원소)부터 새 용량만큼 유지되고 나머지는 버려진다.
    IntBuffer buffer(8);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);
    buffer.Push(4);

    buffer.SetCapacity(3);   // 최대 2개만 보관 가능해짐
    EXPECT_EQ(buffer.GetCapacity(), 3u);
    EXPECT_EQ(buffer.GetSize(), 2u);
    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 1, 2 }));
}

TEST(RingBuffer, SetCapacityShrinkNeverExceedsMaxSizeInvariant)
{
    // 축소 후에도 size + 1 == capacity 이상은 절대 허용되면 안 된다(head==tail 모호성 방지).
    IntBuffer buffer(8);
    for (int i = 0; i < 6; ++i)
    {
        buffer.Push(i);
    }

    buffer.SetCapacity(2);   // 최대 1개만 보관 가능해짐
    EXPECT_EQ(buffer.GetCapacity(), 2u);
    EXPECT_LE(buffer.GetSize(), buffer.GetMaxSize());
    EXPECT_EQ(buffer.GetSize(), 1u);
}

// ==========================================================
//  순회 (head -> tail)
// ==========================================================

TEST(RingBuffer, IterationOrderIsHeadToTail)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    std::vector<int> values;
    for (auto it = buffer.Begin(); it != buffer.End(); ++it)
    {
        values.push_back(*it);
    }

    EXPECT_EQ(values, (std::vector<int>{ 1, 2, 3 }));
}

TEST(RingBuffer, ConstIterationWorks)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);

    const IntBuffer& constBuffer = buffer;
    EXPECT_EQ(ToVector(constBuffer), (std::vector<int>{ 1, 2 }));
}

TEST(RingBuffer, IterationAfterWrapAroundIsStillOrdered)
{
    IntBuffer buffer(4);
    for (int i = 0; i < 6; ++i)
    {
        buffer.Push(i);   // 3, 4, 5만 남음
    }

    EXPECT_EQ(ToVector(buffer), (std::vector<int>{ 3, 4, 5 }));
}

// ==========================================================
//  복사 / 이동
// ==========================================================

TEST(RingBuffer, CopyConstructorDeepCopies)
{
    IntBuffer original(4);
    original.Push(1);
    original.Push(2);

    IntBuffer copy(original);
    copy.Push(3);

    EXPECT_EQ(ToVector(original), (std::vector<int>{ 1, 2 }));
    EXPECT_EQ(ToVector(copy), (std::vector<int>{ 1, 2, 3 }));
}

TEST(RingBuffer, CopyAssignmentDeepCopies)
{
    IntBuffer original(4);
    original.Push(1);
    original.Push(2);

    IntBuffer copy(4);
    copy.Push(99);
    copy = original;
    copy.Push(3);

    EXPECT_EQ(ToVector(original), (std::vector<int>{ 1, 2 }));
    EXPECT_EQ(ToVector(copy), (std::vector<int>{ 1, 2, 3 }));
}

TEST(RingBuffer, MoveConstructorTransfersElements)
{
    IntBuffer original(4);
    original.Push(1);
    original.Push(2);

    IntBuffer moved(std::move(original));
    EXPECT_EQ(ToVector(moved), (std::vector<int>{ 1, 2 }));
    EXPECT_TRUE(original.IsEmpty());   // NOLINT(bugprone-use-after-move) - 이동 후 상태 검증
}

TEST(RingBuffer, MoveAssignmentTransfersElements)
{
    IntBuffer original(4);
    original.Push(1);
    original.Push(2);

    IntBuffer moved(4);
    moved.Push(99);
    moved = std::move(original);

    EXPECT_EQ(ToVector(moved), (std::vector<int>{ 1, 2 }));
    EXPECT_TRUE(original.IsEmpty());   // NOLINT(bugprone-use-after-move)
}

// ==========================================================
//  range-for 지원 (소문자 begin()/end())
// ==========================================================

TEST(RingBuffer, LowercaseBeginEndSupportRangeFor)
{
    IntBuffer buffer(4);
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    std::vector<int> values;
    for (const int value : buffer)   // range-for는 소문자 begin()/end()만 봄
    {
        values.push_back(value);
    }

    EXPECT_EQ(values, (std::vector<int>{ 1, 2, 3 }));
}
