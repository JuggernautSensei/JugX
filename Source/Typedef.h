#pragma once
#include <array>
#include <list>
#include <map>
#include <set>
#include <span>
#include <vector>
#include <filesystem>
#include <chrono>

#include "Vendor/parallel-hashmap/parallel_hashmap/phmap.h"

// ===========================================
//  Shortcut
// ===========================================

// filesystem
using FilePath         = std::filesystem::path;
using DirIter          = std::filesystem::directory_iterator;
using RecursiveDirIter = std::filesystem::recursive_directory_iterator;
using DirEntry         = std::filesystem::directory_entry;
using FileStatus       = std::filesystem::file_status;

// chrono
using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

template<typename T>
using TimePoint = std::chrono::time_point<T>;
using namespace std::chrono_literals;   // NOLINT

using Sec = std::chrono::seconds;
using Ms  = std::chrono::milliseconds;
using Us  = std::chrono::microseconds;
using Ns  = std::chrono::nanoseconds;

// ===========================================
//  Container
// ===========================================

template<typename T, typename TAlloc = std::allocator<T>>
using Vector = std::vector<T, TAlloc>;

template<typename T, size_t N>
using ARRAY = std::array<T, N>;

template<typename T, typename TAlloc = std::allocator<T>>
using List = std::list<T, TAlloc>;

template<typename T, typename TComp = std::less<T>, typename TAlloc = std::allocator<T>>
using RbSet = std::set<T, TComp, TAlloc>;

template<typename TKey, typename TValue, typename TComp = std::less<TKey>, typename TAlloc = std::allocator<std::pair<const TKey, TValue>>>
using RbMap = std::map<TKey, TValue, TComp, TAlloc>;

template<typename TKey, typename TValue, typename THasher = phmap::priv::hash_default_hash<TKey>, typename TEqual = phmap::priv::hash_default_eq<TKey>, typename TAlloc = std::allocator<std::pair<const TKey, TValue>>>
using HashMap = phmap::parallel_flat_hash_map<TKey, TValue, THasher, TEqual, TAlloc>;

template<typename TKey, typename TValue, typename THasher = phmap::priv::hash_default_hash<TKey>, typename TEqual = phmap::priv::hash_default_eq<TKey>, typename TAlloc = std::allocator<std::pair<const TKey, TValue>>>
using HashSet = phmap::parallel_flat_hash_set<TKey, THasher, TEqual, TAlloc>;

template<typename T, size_t kExtent = std::dynamic_extent>
using Span = std::span<T, kExtent>;

using String      = std::string;
using StringView  = std::string_view;
using WString     = std::wstring;
using WStringView = std::wstring_view;
