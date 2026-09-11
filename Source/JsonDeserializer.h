#pragma once
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "Assertion.h"
#include "EnumFlags.h"
#include "EnumRefl.h"
#include "JsonAdlSerializer.h"
#include "Macro.h"
#include "SerializeError.h"
#include "Typedef.h"
#include "TypeTraits.h"

struct yyjson_doc;
struct yyjson_val;

namespace jug
{

class JsonReader;

// ==========================================================
//  Type Traits
// ==========================================================

template<typename T>
concept JsonReadableIntrusiveT = requires(const JsonReader& _reader) { { T::Deserialize(_reader) } -> std::same_as<SerializeResult<T>>; };

template<typename T>
concept JsonReadableAdlT = requires(const JsonReader& _reader) { { JsonAdlSerializer<T>::Deserialize(_reader) } -> std::same_as<SerializeResult<T>>; };

// ==========================================================
//  JsonReader
// ==========================================================

class JsonReader
{
    friend class JsonDeserializer;

public:
    // ===========================================
    //  Array Iterator
    // ===========================================

    class ArrayIterator
    {
        friend class JsonReader;

    public:
        ArrayIterator() = default;

        [[nodiscard]] JsonReader           operator*() const;
        ArrayIterator&                     operator++();
        [[nodiscard]] ArrayIterator        operator++(int);
        [[nodiscard]] bool                 operator==(const ArrayIterator& _other) const;
        [[nodiscard]] std::strong_ordering operator<=>(const ArrayIterator& _other) const;

    private:
        constexpr ArrayIterator(const yyjson_val* _pValue, size_t _remain);

        const yyjson_val* m_pValue = nullptr;
        size_t            m_remain = 0;
    };

    class ArrayRange
    {
        friend class JsonReader;

    public:
        [[nodiscard]] ArrayIterator begin() const;
        [[nodiscard]] ArrayIterator end() const;

    private:
        explicit ArrayRange(ArrayIterator _iter);

        ArrayIterator m_iter = {};
    };

    // ===========================================
    //  Object Iterator
    // ===========================================

    class ObjectIterator
    {
        friend class JsonReader;

    public:
        ObjectIterator() = default;

        [[nodiscard]] std::pair<StringView, JsonReader> operator*() const;
        ObjectIterator&                                 operator++();
        [[nodiscard]] ObjectIterator                    operator++(int);
        [[nodiscard]] bool                              operator==(const ObjectIterator& _other) const;
        [[nodiscard]] std::strong_ordering              operator<=>(const ObjectIterator& _other) const;

    private:
        constexpr ObjectIterator(const yyjson_val* _pKey, size_t _remain);

        const yyjson_val* m_pKey   = nullptr;
        size_t            m_remain = 0;
    };

    class ObjectRange
    {
        friend class JsonReader;

    public:
        [[nodiscard]] ObjectIterator begin() const;
        [[nodiscard]] ObjectIterator end() const;

    private:
        explicit ObjectRange(ObjectIterator _iter);

        ObjectIterator m_iter = {};
    };

public:
    // ======================================
    //  Read
    // ======================================

    template<typename T>
    [[nodiscard]] SerializeResult<T> Read() const
    {
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "T must be a plain type (no const/reference)");

        if constexpr (JsonReadableIntrusiveT<T>)
        {
            return T::Deserialize(*this);
        }
        else if constexpr (JsonReadableAdlT<T>)
        {
            return JsonAdlSerializer<T>::Deserialize(*this);
        }
        else
        {
            return Read_(std::type_identity<T> {});
        }
    }

    template<typename T>
    [[nodiscard]] SerializeResult<T> ReadField(
        const StringView _key) const
    {
        SerializeResult<JsonReader> reader = FindField(_key);
        JUG_DISPATCH_FAILED(reader);
        return reader->Read<T>();
    }

    [[nodiscard]] SerializeResult<JsonReader> FindField(StringView _key) const;

    // ======================================
    //  Iteration
    // ======================================

    [[nodiscard]] ArrayIterator BeginArray() const;
    [[nodiscard]] ArrayIterator EndArray() const;
    [[nodiscard]] ArrayRange    GetArray() const;

    [[nodiscard]] ObjectIterator BeginObject() const;
    [[nodiscard]] ObjectIterator EndObject() const;
    [[nodiscard]] ObjectRange    GetObject() const;

    // ======================================
    //  Utils
    // ======================================

    [[nodiscard]] bool   HasField(StringView _key) const;
    [[nodiscard]] size_t GetSize() const;

    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsBool() const;
    [[nodiscard]] bool IsNumber() const;
    [[nodiscard]] bool IsString() const;
    [[nodiscard]] bool IsArray() const;
    [[nodiscard]] bool IsObject() const;
    [[nodiscard]] bool IsContainer() const;

private:
    explicit JsonReader(const yyjson_val* _pValue);

    [[nodiscard]] SerializeResult<bool>        Read_(std::type_identity<bool>) const;
    [[nodiscard]] SerializeResult<int64_t>     Read_(std::type_identity<int64_t>) const;
    [[nodiscard]] SerializeResult<uint64_t>    Read_(std::type_identity<uint64_t>) const;
    [[nodiscard]] SerializeResult<double>      Read_(std::type_identity<double>) const;
    [[nodiscard]] SerializeResult<StringView>  Read_(std::type_identity<StringView>) const;
    [[nodiscard]] SerializeResult<String>      Read_(std::type_identity<String>) const;
    [[nodiscard]] SerializeResult<const char*> Read_(std::type_identity<const char*>) const;

    template<EnumT E>
    [[nodiscard]] SerializeResult<E> Read_(
        std::type_identity<E>) const
    {
        using U                   = UnderlyingT<E>;
        SerializeResult<U> result = Read_(std::type_identity<U> {});
        JUG_DISPATCH_FAILED(result);
        return static_cast<E>(result.GetValue());
    }

    template<std::floating_point T>
    [[nodiscard]] SerializeResult<T> Read_(
        std::type_identity<T>) const
    {
        SerializeResult<double> result = Read_(std::type_identity<double> {});
        JUG_DISPATCH_FAILED(result);
        return static_cast<T>(result.GetValue());
    }

    template<std::signed_integral T>
    [[nodiscard]] SerializeResult<T> Read_(
        std::type_identity<T>) const
    {
        SerializeResult<int64_t> result = Read_(std::type_identity<int64_t> {});
        JUG_DISPATCH_FAILED(result);
        return static_cast<T>(result.GetValue());
    }

    template<std::unsigned_integral T>
    [[nodiscard]] SerializeResult<T> Read_(
        std::type_identity<T>) const
    {
        SerializeResult<uint64_t> result = Read_(std::type_identity<uint64_t> {});
        JUG_DISPATCH_FAILED(result);
        return static_cast<T>(result.GetValue());
    }

    const yyjson_val* m_pValue = nullptr;
};

constexpr JsonReader::ArrayIterator::ArrayIterator(
    const yyjson_val* const _pValue,
    const size_t            _remain)
    : m_pValue(_pValue)
    , m_remain(_remain)
{
}

constexpr JsonReader::ObjectIterator::ObjectIterator(
    const yyjson_val* const _pKey,
    const size_t            _remain)
    : m_pKey(_pKey)
    , m_remain(_remain)
{
}

// ==========================================================
//  JsonDeserializer
// ==========================================================

enum class eJsonLoadOption : uint32_t
{
    None                 = 0,
    InSitu               = 1 << 0,
    StopWhenDone         = 1 << 1,
    AllowTrailingCommas  = 1 << 2,
    AllowComments        = 1 << 3,
    AllowInfAndNan       = 1 << 4,
    NumberAsRaw          = 1 << 5,
    AllowInvalidUnicode  = 1 << 6,
    BigNumAsRaw          = 1 << 7,
    AllowBOM             = 1 << 8,
    AllowExtNumber       = 1 << 9,
    AllowExtEscape       = 1 << 10,
    AllowExtWhitespace   = 1 << 11,
    AllowSingleQuotedStr = 1 << 12,
    AllowUnquotedKey     = 1 << 13,
};

class JsonDeserializer
{
public:
    [[nodiscard]] static SerializeResult<JsonDeserializer> LoadFromString(StringView _json, Flags<eJsonLoadOption> _options = eJsonLoadOption::None);
    [[nodiscard]] static SerializeResult<JsonDeserializer> LoadFromFile(const FilePath& _path, Flags<eJsonLoadOption> _options = eJsonLoadOption::None);

    ~JsonDeserializer();
    JsonDeserializer(const JsonDeserializer&)            = delete;
    JsonDeserializer& operator=(const JsonDeserializer&) = delete;
    JsonDeserializer(JsonDeserializer&& _other) noexcept;
    JsonDeserializer& operator=(JsonDeserializer&& _other) noexcept;

    [[nodiscard]] JsonReader GetReader() const;

private:
    explicit JsonDeserializer(yyjson_doc* _pDoc);

    yyjson_doc* m_pDoc = nullptr;
};

}   // namespace jug
