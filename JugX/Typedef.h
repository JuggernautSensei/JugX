#pragma once
#include <array>
#include <chrono>
#include <filesystem>
#include <list>
#include <map>
#include <set>
#include <span>
#include <vector>
#include <parallel_hashmap/phmap.h>

namespace jug
{

// ===========================================
//  FileSystem
// ===========================================

using FilePath         = std::filesystem::path;
using DirIter          = std::filesystem::directory_iterator;
using RecursiveDirIter = std::filesystem::recursive_directory_iterator;
using DirEntry         = std::filesystem::directory_entry;
using FileStatus       = std::filesystem::file_status;
namespace fs           = std::filesystem;

// ===========================================
//  Chrono
// ===========================================

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

template<typename T>
using Allocator = std::allocator<T>;

template<typename T, typename Alloc = Allocator<T>>
using Vector = std::vector<T, Alloc>;

template<typename T, size_t kSize>
using ARRAY = std::array<T, kSize>;

template<typename T, typename Alloc = Allocator<T>>
using List = std::list<T, Alloc>;

template<typename T, typename Cmp = std::less<T>, typename Alloc = Allocator<T>>
using RbSet = std::set<T, Cmp, Alloc>;

template<typename K, typename V, typename Cmp = std::less<K>, typename Alloc = Allocator<std::pair<const K, V>>>
using RbMap = std::map<K, V, Cmp, Alloc>;

template<typename T>
using Hasher = phmap::priv::hash_default_hash<T>;

template<typename T>
using EqualTo = phmap::priv::hash_default_eq<T>;

template<typename K, typename V, typename H = Hasher<K>, typename E = EqualTo<K>, typename Alloc = Allocator<std::pair<const K, V>>>
using HashMap = phmap::parallel_flat_hash_map<K, V, H, E, Alloc>;

template<typename K, typename V, typename H = Hasher<K>, typename E = EqualTo<K>, typename Alloc = Allocator<std::pair<const K, V>>>
using HashSet = phmap::parallel_flat_hash_set<K, H, E, Alloc>;

template<typename T, size_t kExtent = std::dynamic_extent>
using Span = std::span<T, kExtent>;

template<typename T>
using BasicString = std::basic_string<T>;

template<typename T>
using BasicStringView = std::basic_string_view<T>;

using String      = BasicString<char>;
using StringView  = BasicStringView<char>;
using WString     = BasicString<wchar_t>;
using WStringView = BasicStringView<wchar_t>;

}   // namespace jug