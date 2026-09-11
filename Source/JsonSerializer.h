#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "Assertion.h"
#include "EnumFlags.h"
#include "EnumRefl.h"
#include "JsonAdlSerializer.h"
#include "Macro.h"
#include "SerializeError.h"
#include "Typedef.h"
#include "TypeTraits.h"

struct yyjson_mut_doc;
struct yyjson_mut_val;

namespace jug
{

class JsonWriter;

// ==========================================================
//  Type Traits
// ==========================================================

template<typename T>
concept JsonSerializableIntrusiveT = requires(const T& _value, JsonWriter& _writer) { { _value.Serialize(_writer) } -> std::same_as<void>; };

template<typename T>
concept JsonSerializableAdlT = requires(JsonWriter& _writer, const T& _value) { { JsonAdlSerializer<T>::Serialize(_writer, _value) } -> std::same_as<void>; };

// ==========================================================
//  JsonWriter
// ==========================================================

class JsonWriter
{
    friend class JsonSerializer;

public:
    // ===========================================
    //  Container Type
    // ===========================================

    void SetObject();
    void SetArray();

    // ===========================================
    //  Write
    // ===========================================

    template<typename T>
    void Write(
        const T& _value)
    {
        if constexpr (JsonSerializableIntrusiveT<T>)
        {
            _value.Serialize(*this);
        }
        else if constexpr (JsonSerializableAdlT<T>)
        {
            JsonAdlSerializer<T>::Serialize(*this, _value);
        }
        else
        {
            Write_(_value);
        }
    }

    template<typename T>
    void WriteField(
        const StringView _key,
        const T&         _value)
    {
        JUG_ASSERT(IsObject(), "WriteField requires an object value - call SetObject() first");

        JsonWriter writer { m_pDoc, nullptr };
        writer.Write(_value);
        JUG_ASSERT(writer.m_pValue, "WriteField: the serializer of T did not write any value");

        AddField_(_key, writer.m_pValue);
    }

    template<typename T>
    void PushBack(
        const T& _value)
    {
        JUG_ASSERT(IsArray(), "PushBack requires an array value - call SetArray() first");

        JsonWriter writer { m_pDoc, nullptr };
        writer.Write(_value);
        JUG_ASSERT(writer.m_pValue, "PushBack: the serializer of T did not write any value");

        AddElement_(writer.m_pValue);
    }

    // ===========================================
    //  Nested Containers
    // ===========================================

    [[nodiscard]] JsonWriter BeginObjectField(StringView _key) const;
    [[nodiscard]] JsonWriter BeginArrayField(StringView _key) const;
    [[nodiscard]] JsonWriter PushBackObject() const;
    [[nodiscard]] JsonWriter PushBackArray() const;

    // ===========================================
    //  Utils
    // ===========================================

    [[nodiscard]] bool IsObject() const;
    [[nodiscard]] bool IsArray() const;

private:
    JsonWriter(yyjson_mut_doc* _pDoc, yyjson_mut_val* _pValue);

    void SetValue_(yyjson_mut_val* _pValue);
    void AddField_(StringView _key, yyjson_mut_val* _pValue) const;
    void AddElement_(yyjson_mut_val* _pValue) const;

    void Write_(std::nullptr_t);
    void Write_(bool _value);
    void Write_(int64_t _value);
    void Write_(uint64_t _value);
    void Write_(double _value);
    void Write_(const String& _value);
    void Write_(StringView _value);
    void Write_(const char* _value);

    template<EnumT E>
    void Write_(const E _value)
    {
        using U = UnderlyingT<E>;
        Write_(static_cast<U>(_value));
    }

    template<std::floating_point T>
    void Write_(const T _value)
    {
        Write_(static_cast<double>(_value));
    }

    template<std::signed_integral T>
    void Write_(const T _value)
    {
        Write_(static_cast<int64_t>(_value));
    }

    template<std::unsigned_integral T>
    void Write_(const T _value)
    {
        Write_(static_cast<uint64_t>(_value));
    }

    yyjson_mut_doc* m_pDoc   = nullptr;
    yyjson_mut_val* m_pValue = nullptr;
};

// ==========================================================
//  JsonSerializer
// ==========================================================

enum class eJsonSaveOption : uint32_t
{
    None                = 0,
    Pretty              = 1 << 0,
    EscapeUnicode       = 1 << 1,
    EscapeSlashes       = 1 << 2,
    AllowInfAndNan      = 1 << 3,
    InfAndNanAsNull     = 1 << 4,
    AllowInvalidUnicode = 1 << 5,
    PrettyTwoSpaces     = 1 << 6,
    NewlineAtEnd        = 1 << 7,
    LowercaseHex        = 1 << 8,
};

class JsonSerializer
{
public:
    JsonSerializer();

    ~JsonSerializer();
    JsonSerializer(const JsonSerializer&)            = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;
    JsonSerializer(JsonSerializer&& _other) noexcept;
    JsonSerializer& operator=(JsonSerializer&& _other) noexcept;

    [[nodiscard]] SerializeResult<String> SaveToString(Flags<eJsonSaveOption> _options = eJsonSaveOption::None) const;
    [[nodiscard]] eSerializerError        SaveToFile(const FilePath& _path, Flags<eJsonSaveOption> _options = eJsonSaveOption::None) const;

    [[nodiscard]] JsonWriter GetWriter() const;

private:
    yyjson_mut_doc* m_pDoc = nullptr;
};

}   // namespace jug
