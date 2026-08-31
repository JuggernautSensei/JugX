#pragma once
// TODO..
// #include <cstdint>
// #include <memory>
// #include <unordered_map>
// #include <vector>
//
//
// #include "AABB.h"
// #include "EnumRefl.h"
// #include "ObjectPool.h"
//
// #define DT octree_detail
//
// namespace jug
//{
//
// namespace DT
//{
//
//    enum class eOctant : uint8_t
//    {
//        LeftBottomBack   = 0b000,   // -X, -Y, -Z
//        LeftBottomFront  = 0b001,   // -X, -Y, +Z
//        LeftTopBack      = 0b010,   // -X, +Y, -Z
//        LeftTopFront     = 0b011,   // -X, +Y, +Z
//        RightBottomBack  = 0b100,   // +X, -Y, -Z
//        RightBottomFront = 0b101,   // +X, -Y, +Z
//        RightTopBack     = 0b110,   // +X, +Y, -Z
//        RightTopFront    = 0b111    // +X, +Y, +Z
//    };
//
//    using OctantMask = UnderlyingT<eOctant>;
//
//}   // namespace DT
//
//// =======================================================
////  Octree
//// =======================================================
//
// struct OctreeDesc
//{
//    float looseFactor               = 1.5f;
//    int   splitThreshold            = 8;
//    int   maxDepth                  = 8;
//    bool  bAllowExpend              = true;
//    bool  bIncreaseMaxDepthOnExpend = true;
//};
//
//// template<
////     typename TKey,
////     typename TValue,
////     typename TKeyEqual = std::equal_to<TKey>,
////     template<typename ...> class TVector = Vector,
////     template<typename ...> class TUnorderedMap = std::unordered_map>
// class Octree
//{
//     struct Item;
//     struct Node;
//
//     using TKey       = int;
//     using TValue     = int;
//     using TKeyHasher = std::hash<TKey>;
//     using TKeyEqual  = std::equal_to<TKey>;
//
//     template<typename T>
//     using TVector = Vector<T>;
//
//     template<typename K, typename V, typename H = TKeyHasher, typename E = TKeyEqual>
//     using TUnorderedMap = std::unordered_map<K, V, H, E>;
//
//     struct Item
//     {
//         AABB   bound = Zero<AABB>();
//         TKey   key;
//         TValue value;
//         Node*  pNode;
//     };
//
//     struct Node
//     {
//         AABB                           bound;
//         TVector<Item*>                 items;
//         ENUM_ARRAY<DT::eOctant, Node*> children;
//         Node*                          pParent;
//     };
//
//     using ScopedItem = std::unique_ptr<Item, ObjectPoolDeleter<Item>>;
//     using ScopedNode = std::unique_ptr<Node, ObjectPoolDeleter<Node>>;
//
// public:
//     void Insert(
//         const TKey&   _key,
//         const TValue& _value,
//         const AABB&   _worldBound)
//     {
//         JUG_ASSERT(!Contains(_key), "Octree: key already exists");
//     }
//
//     void Remove(
//         const TKey&   _key,
//         const TValue& _value)
//     {
//     }
//
//     void Update(
//         const TKey& _key,
//         const AABB& _bound)
//     {
//     }
//
//     bool Contains(
//         const TKey& _key) const
//     {
//         return m_map.contains(_key);
//     }
//
// private:
//     void Insert_Recursive_()
//     {
//     }
//
//     /*  [[nodiscard]] Node* FindBestChildOctantOrNull_(
//           Node* _pNode,
//           Item* _pItem)
//       {
//       }*/
//
//     [[no_unique_address]] TKeyEqual                   m_equal    = {};
//     OctreeDesc                                        m_desc     = {};
//     TUnorderedMap<TKey, Item*, TKeyHasher, TKeyEqual> m_map      = {};
//     ObjectPool<Item>                                  m_itemPool = {};
//     ObjectPool<Node>                                  m_nodePool = {};
// };
//
// }   // namespace jug
//
// #undef DT
