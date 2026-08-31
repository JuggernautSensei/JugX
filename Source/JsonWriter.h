// C++20 unified serialization library providing serialization/deserialization in various formats
#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "Config.h"
#include "Macros.h"
#include "SerializerError.h"
#include "TypeTraits.h"
#include "EnumFlags.h"
#include "Vendor/yyjson/src/yyjson.h"

struct yyjson_mut_doc;
struct yyjson_mut_val;

namespace jug
{

class JsonWriter;

// =========================================================
//  Json Writer Type Traits
// =========================================================

template<typename T>
concept JsonWriteableByMethodT = requires(const T& _value, JsonWriter& _writer) { { _value.Serialize(_writer) } -> std::same_as<void>; };

template<typename T>
concept JsonWriteableByFuncT = requires(JsonWriter& _writer, const T& _value) { { Serialize(_writer, _value) } -> std::same_as<void>; };

template<typename T>
concept JsonWriteableByCustomT = JsonWriteableByMethodT<T> || JsonWriteableByFuncT<T>;

// ==========================================================
//  JsonWriter
// ==========================================================

enum class eJsonWriteOption : uint32_t
{
    None                = 0,
    Pretty              = 1 << 0,   // WriteV JSON pretty with 4 space indent.
    EscapeUnicode       = 1 << 1,   // Escape unicode as `uXXXX`, make the output ASCII only.
    EscapeSlashes       = 1 << 2,   // Escape '/' as '\/'.
    AllowInfAndNan      = 1 << 3,   // WriteV inf and nan number as 'Infinity' and 'NaN' literal.
    InfAndNanAsNull     = 1 << 4,   // WriteV inf and nan number as null literal. Overrides AllowInfAndNan.
    AllowInvalidUnicode = 1 << 5,   // Allow invalid unicode when encoding string values.
    PrettyTwoSpaces     = 1 << 6,   // WriteV JSON pretty with 2 space indent. Overrides Pretty.
    NewlineAtEnd        = 1 << 7,   // Adds a newline character `\n` at the end of the JSON.
    LowercaseHex        = 1 << 8,   // Use lowercase hex digits in `\uXXXX`. Only with EscapeUnicode.
};

class JsonWriter
{
    struct Frame
    {
        yyjson_mut_val* pContainer = nullptr;
        bool            bObject    = false;
    };

public:
    JsonWriter();
    ~JsonWriter();

    JsonWriter(const JsonWriter&)            = delete;
    JsonWriter& operator=(const JsonWriter&) = delete;
    JsonWriter(JsonWriter&& _other) noexcept;
    JsonWriter& operator=(JsonWriter&& _other) noexcept;

    // ======================================
    //  WriteV
    // ======================================

    template<typename T>
    void Write(
        const T& _value)
    {
        Write_(_value);
    }

    template<typename T>
    void WriteField(
        const StringView _key,
        const T&               _value)
    {
        WriteKey(_key);
        Write_(_value);
    }

    // ======================================
    //  Object
    // ======================================

    void BeginObject();
    void BeginObject(StringView _key);
    void EndObject();

    void WriteKey(StringView _key);

    // ======================================
    //  Array
    // ======================================

    void BeginArray();
    void BeginArray(StringView _key);
    void EndArray();

    // ======================================
    //  Output
    // ======================================

    [[nodiscard]] SerializerResult<String> SaveToString(Flags<eJsonWriteOption> _options = eJsonWriteOption::None) const;
    eSerializerError                            SaveToFile(const FilePath& _path, Flags<eJsonWriteOption> _options = eJsonWriteOption::None) const;

private:
    void AttachValue_(yyjson_mut_val* _pValue);
    void PushFrame_(bool _bObject);

    // ===========================================
    //  WriteV Pritmive
    // ===========================================

    void Write_(std::nullptr_t);
    void Write_(bool _value);
    void Write_(int64_t _value);
    void Write_(uint64_t _value);
    void Write_(double _value);
    void Write_(StringView _value);
    void Write_(const char* _value);

    template<typename T>
        requires std::is_enum_v<T>
    void Write_(
        const T _value)
    {
        using U = std::underlying_type_t<T>;
        Write_(static_cast<U>(_value));
    }

    template<std::signed_integral T>
    void Write_(
        const T _value)
    {
        Write_(static_cast<int64_t>(_value));
    }

    template<std::unsigned_integral T>
    void Write_(
        const T _value)
    {
        Write_(static_cast<uint64_t>(_value));
    }

    template<std::floating_point T>
    void Write_(
        const T _value)
    {
        Write_(static_cast<double>(_value));
    }

    // ===========================================
    //  WriteV Custom
    // ===========================================

    template<JsonWriteableByCustomT T>
    void Write_(
        const T& _value)
    {
        if constexpr (JsonWriteableByMethodT<T>)
        {
            _value.Serialize(*this);
        }
        else
        {
            Serialize(*this, _value);
        }
    }

    yyjson_mut_doc*    m_pDoc        = nullptr;
    yyjson_mut_val*    m_pPendingKey = nullptr;
    Vector<Frame> m_frameStack  = {};
};

}   // namespace jug
