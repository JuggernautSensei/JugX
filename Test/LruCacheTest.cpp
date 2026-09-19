// LruCache 동작 검증용 테스트 (Google Test)
//
// LRU 정책: Find/Touch/At/operator[]는 해당 원소를 MRU(most-recently-used)로 승격시킨다.
// Peek은 승격 없이 조회만 한다. 용량 초과 시 가장 오래 안 쓰인(LRU) 원소부터 제거된다.
// 순회 순서(Begin~End, begin~end)는 항상 MRU -> LRU 순서다.
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <JugX/LruCache.h>

using namespace jug;

namespace
{
using IntCache = LruCache<int, std::string>;
}

// ==========================================================
//  Insert / Emplace
// ==========================================================

TEST(LruCache, InsertNewKeySucceeds)
{
    IntCache cache(4);

    const auto [it, inserted] = cache.Insert(1, "a");
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it->second, "a");
    EXPECT_EQ(cache.GetSize(), 1u);
}

TEST(LruCache, InsertExistingKeyFailsAndTouches)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");

    // 1을 다시 insert하면 실패하지만 MRU로 승격되어야 한다.
    const auto [it, inserted] = cache.Insert(1, "z");
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it->second, "a");   // 값은 바뀌지 않는다.

    EXPECT_EQ(cache.Begin()->first, 1);
}

TEST(LruCache, EmplaceConstructsInPlace)
{
    LruCache<int, std::string> cache(4);
    const auto [it, inserted] = cache.Emplace(1, 3, 'x');   // std::string(3, 'x')
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it->second, "xxx");
}

// ==========================================================
//  InsertOrAssign
// ==========================================================

TEST(LruCache, InsertOrAssignInsertsWhenMissing)
{
    IntCache cache(4);
    const auto it = cache.InsertOrReplace(1, std::string("a"));
    EXPECT_EQ(it->second, "a");
    EXPECT_EQ(cache.GetSize(), 1u);
}

TEST(LruCache, InsertOrAssignOverwritesWhenPresent)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    const auto it = cache.InsertOrReplace(1, std::string("b"));
    EXPECT_EQ(it->second, "b");
    EXPECT_EQ(cache.GetSize(), 1u);
}

// ==========================================================
//  Find / Peek / At / operator[]
// ==========================================================

TEST(LruCache, FindTouchesAndPromotesToMru)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");

    // 3, 2, 1 순서(MRU->LRU)에서 1을 Find하면 1이 맨 앞으로 온다.
    const auto it = cache.Find(1);
    ASSERT_NE(it, cache.End());
    EXPECT_EQ(it->second, "a");
    EXPECT_EQ(cache.Begin()->first, 1);
}

TEST(LruCache, FindMissingKeyReturnsEnd)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    EXPECT_EQ(cache.Find(99), cache.End());
}

TEST(LruCache, PeekDoesNotPromote)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");

    const auto it = cache.Peek(1);
    ASSERT_NE(it, cache.End());
    EXPECT_EQ(it->second, "a");

    // 순서는 그대로: 3, 2, 1
    EXPECT_EQ(cache.Begin()->first, 3);
}

TEST(LruCache, AtTouchesAndReturnsMutableRef)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.At(1) = "z";
    EXPECT_EQ(cache.Peek(1)->second, "z");
}

TEST(LruCache, IndexOperatorDefaultConstructsMissingKey)
{
    IntCache cache(4);
    EXPECT_TRUE(cache[1].empty());
    cache[1] = "a";
    EXPECT_EQ(cache.Peek(1)->second, "a");
}

// ==========================================================
//  용량 / 축출(eviction)
// ==========================================================

TEST(LruCache, EvictsLeastRecentlyUsedWhenOverCapacity)
{
    IntCache cache(2);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");   // 1이 LRU이므로 축출되어야 한다.

    EXPECT_EQ(cache.GetSize(), 2u);
    EXPECT_FALSE(cache.Contains(1));
    EXPECT_TRUE(cache.Contains(2));
    EXPECT_TRUE(cache.Contains(3));
}

TEST(LruCache, TouchingKeepsItFromEviction)
{
    IntCache cache(2);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Touch(1);         // 순서: 1, 2
    cache.Insert(3, "c");   // 2가 LRU이므로 축출되어야 한다.

    EXPECT_TRUE(cache.Contains(1));
    EXPECT_FALSE(cache.Contains(2));
    EXPECT_TRUE(cache.Contains(3));
}

TEST(LruCache, EraseOldestRemovesLruElement)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");

    cache.EraseOldest();   // 1이 LRU
    EXPECT_FALSE(cache.Contains(1));
    EXPECT_EQ(cache.GetSize(), 2u);
}

TEST(LruCache, SetCapacityShrinksAndTrimsExcess)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");
    cache.Insert(4, "d");   // 순서: 4, 3, 2, 1

    cache.SetCapacity(2);
    EXPECT_EQ(cache.GetSize(), 2u);
    EXPECT_EQ(cache.GetCapacity(), 2u);
    EXPECT_TRUE(cache.Contains(4));
    EXPECT_TRUE(cache.Contains(3));
    EXPECT_FALSE(cache.Contains(2));
    EXPECT_FALSE(cache.Contains(1));
}

// ==========================================================
//  Erase / Clear / 상태 조회
// ==========================================================

TEST(LruCache, EraseExistingKeyReturnsTrue)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    EXPECT_TRUE(cache.Erase(1));
    EXPECT_FALSE(cache.Contains(1));
    EXPECT_TRUE(cache.IsEmpty());
}

TEST(LruCache, EraseMissingKeyReturnsFalse)
{
    IntCache cache(4);
    EXPECT_FALSE(cache.Erase(99));
}

TEST(LruCache, ClearRemovesAllElements)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Clear();

    EXPECT_TRUE(cache.IsEmpty());
    EXPECT_EQ(cache.GetSize(), 0u);
    EXPECT_FALSE(cache.Contains(1));
}

TEST(LruCache, DefaultCapacityIsEight)
{
    IntCache cache;
    EXPECT_EQ(cache.GetCapacity(), 8u);
}

// ==========================================================
//  순회 (MRU -> LRU)
// ==========================================================

TEST(LruCache, IterationOrderIsMruToLru)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");
    cache.Insert(3, "c");   // 순서: 3, 2, 1

    std::vector<int> keys;
    for (const auto& [key, value] : cache)
    {
        keys.push_back(key);
    }

    EXPECT_EQ(keys, (std::vector<int>{ 3, 2, 1 }));
}

TEST(LruCache, ConstIterationWorks)
{
    IntCache cache(4);
    cache.Insert(1, "a");
    cache.Insert(2, "b");

    const IntCache& constCache = cache;
    std::vector<int> keys;
    for (const auto& [key, value] : constCache)
    {
        keys.push_back(key);
    }

    EXPECT_EQ(keys, (std::vector<int>{ 2, 1 }));
}