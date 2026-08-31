#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "Config.h"
#include "EnumFlags.h"
#include "SerializerError.h"
#include "TypeTraits.h"

struct yyjson_doc;
struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

namespace jug
{

class JsonReader;
class JsonWriter;

template<typename T>
struct JsonTypeTag
{
};

template<typename T>
constexpr JsonTypeTag<T> kJsonTypeTag = {};

// =========================================================
//  Type Traits
// =========================================================

template<typename T>
concept JsonReaderableByMethodT = !std::is_const_v<T> && requires(T& _value, JsonReader& _reader) {{ _value.Deserialize(_reader) } -> std::same_as<void>; };

template<typename T>
concept JsonReaderableByFuncT = !std::is_const_v<T> && requires(JsonReader& _reader, T& _value) { { Deserialize(_reader, _value) } -> std::same_as<void>; };

// for non-default-constructible types or private constructors
template<typename T, typename U = std::remove_cvref_t<T>>
concept JsonReaderableByFactoryMethodT = !std::is_const_v<T> && requires(JsonReader& _reader) { { U::Deserialize(_reader) } -> std::same_as<U>; };

// for non-default-constructible types
template<typename T>
concept JsonReaderableByFactoryFuncT = !std::is_const_v<T> && requires(JsonReader& _reader) { { Deserialize(_reader, kJsonTypeTag<T>) } -> std::same_as<T>; };

template<typename T>
concept JsonReaderableByFactoryT = JsonReaderableByFactoryMethodT<T> || JsonReaderableByFactoryFuncT<T>;

template<typename T>
concept JsonReaderableByCustomT = JsonReaderableByMethodT<T> || JsonReaderableByFuncT<T> || JsonReaderableByFactoryMethodT<T> || JsonReaderableByFactoryFuncT<T>;

// ==========================================================
//  JsonReader
// ==========================================================

enum class eJsonReadOption : uint32_t
{
    None                 = 0,
    InSitu               = 1 << 0,    // ReadTo the input data in-situ, allowing the reader to modify and use input data to store string values.
    StopWhenDone         = 1 << 1,    // Stop when done instead of issuing an error if there's additional content after a JSON document.
    AllowTrailingCommas  = 1 << 2,    // Allow single trailing comma at the end of an object or array.
    AllowComments        = 1 << 3,    // Allow C-style single-line and multi-line comments.
    AllowInfAndNan       = 1 << 4,    // Allow inf/nan number and literal, case-insensitive.
    NumberAsRaw          = 1 << 5,    // ReadTo all numbers as raw strings (value with `YYJSON_TYPE_RAW` type).
    AllowInvalidUnicode  = 1 << 6,    // Allow reading invalid unicode when parsing string values.
    BigNumAsRaw          = 1 << 7,    // ReadTo big numbers as raw strings.
    AllowBOM             = 1 << 8,    // Allow UTF-8 BOM and skip it before parsing if any.
    AllowExtNumber       = 1 << 9,    // Allow extended number formats.
    AllowExtEscape       = 1 << 10,   // Allow extended escape sequences in strings.
    AllowExtWhitespace   = 1 << 11,   // Allow extended whitespace characters.
    AllowSingleQuotedStr = 1 << 12,   // Allow strings enclosed in single quotes.
    AllowUnquotedKey     = 1 << 13,   // Allow object keys without quotes.
};

class JsonReader
{
    struct Frame
    {
        bool TryNext();

        yyjson_val* pContainer = nullptr;
        yyjson_val* pKey       = nullptr;
        yyjson_val* pValue     = nullptr;
        bool        bObject    = false;
        size_t      index      = 0;
        size_t      size       = 0;
    };

public:
    ~JsonReader();

    JsonReader(const JsonReader&)            = delete;
    JsonReader& operator=(const JsonReader&) = delete;
    JsonReader(JsonReader&& _other) noexcept;
    JsonReader& operator=(JsonReader&& _other) noexcept;

    // ======================================
    //  Factory
    // ======================================

    [[nodiscard]] static SerializerResult<JsonReader> LoadFromString(StringView _json, Flags<eJsonReadOption> _options = eJsonReadOption::None);
    [[nodiscard]] static SerializerResult<JsonReader> LoadFromFile(const FilePath& _path, Flags<eJsonReadOption> _options = eJsonReadOption::None);

    // ======================================
    //  ReadTo
    // ======================================

    template<typename T>
    bool ReadTo(
        T& _outValue)
    {
        return ReadTo_(GetValue_(true), _outValue, true);
    }

    template<typename T>
    bool ReadFieldTo(
        const StringView _key,
        T&               _outValue)
    {
        yyjson_val* pValue = FindFieldOrNull_(_key, true);
        if (!pValue)
        {
            return false;
        }
        return ReadTo_(pValue, _outValue, true);
    }

    // ======================================
    //  Read
    // ======================================

    template<typename T>
    [[nodiscard]] T Read()
    {
        return Read_<T>(GetValue_(true), true);
    }

    template<typename T>
        requires std::is_default_constructible_v<T>
    [[nodiscard]] T ReadField(
        const StringView _key)
    {
        yyjson_val* pValue = FindFieldOrNull_(_key, true);
        if (!pValue)
        {
            return T {};
        }
        return Read_<T>(pValue, true);
    }

    // ======================================
    //  Object
    // ======================================

    bool BeginObject();
    bool BeginObject(StringView _key);
    void EndObject();

    [[nodiscard]] bool       HasField(StringView _key) const;
    [[nodiscard]] StringView GetKey() const;

    // ======================================
    //  Array
    // ======================================

    bool BeginArray();
    bool BeginArray(StringView _key);
    void EndArray();

    // ======================================
    //  For Container
    // ======================================

    void                 Next();
    bool                 HasNext() const;
    [[nodiscard]] size_t GetSize() const;

    // ======================================
    //  Utils
    // ======================================

    [[nodiscard]] bool IsNull();
    [[nodiscard]] bool IsArray();
    [[nodiscard]] bool IsObject();
    [[nodiscard]] bool IsContainer();
    [[nodiscard]] bool IsValue();

    // ======================================
    //  Error
    // ======================================

    [[nodiscard]] bool             HasError() const;
    [[nodiscard]] eSerializerError GetLastError() const;
    [[nodiscard]] std::string_view GetLastErrorMsg() const;

private:
    JsonReader() = default;
    eSerializerError InitFromString_(StringView _json, Flags<eJsonReadOption> _options);
    eSerializerError InitFromFile_(const FilePath& _path, Flags<eJsonReadOption> _options);

    [[nodiscard]] yyjson_val* GetValue_(bool _bNext);
    [[nodiscard]] yyjson_val* FindFieldOrNull_(StringView _key, bool _bCheckError);
    void                      PushFrame_(yyjson_val* _pValue, bool _bObject);
    void                      SetError_(eSerializerError _error, std::string_view _msg);

    // ===========================================
    //  ReadTo Primitive
    // ===========================================

    bool ReadTo_(const yyjson_val* _pValue, bool& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, int64_t& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, uint64_t& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, double& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, StringView& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, String& _outValue, bool _bCheckError);
    bool ReadTo_(const yyjson_val* _pValue, const char*& _outValue, bool _bCheckError);

    template<EnumT E>
    bool ReadTo_(
        const yyjson_val* _pValue,
        E&                _outValue,
        bool              _bCheckError)
    {
        UnderlyingT<E> value;
        const bool     bSucceeded = ReadTo_(_pValue, value, _bCheckError);
        _outValue                 = static_cast<E>(value);
        return bSucceeded;
    }

    template<std::signed_integral T>
    bool ReadTo_(
        const yyjson_val* _pValue,
        T&                _outValue,
        const bool        _bCheckError)
    {
        int64_t    value;
        const bool bSucceeded = ReadTo_(_pValue, value, _bCheckError);
        _outValue             = static_cast<T>(value);
        return bSucceeded;
    }

    template<std::unsigned_integral T>
    bool ReadTo_(
        const yyjson_val* _pValue,
        T&                _outValue,
        const bool        _bCheckError)
    {
        uint64_t   value;
        const bool bSucceeded = ReadTo_(_pValue, value, _bCheckError);
        _outValue             = static_cast<T>(value);
        return bSucceeded;
    }

    template<std::floating_point T>
    bool ReadTo_(
        const yyjson_val* _pValue,
        T&                _outValue,
        const bool        _bCheckError)
    {
        double     value;
        const bool bSucceeded = ReadTo_(_pValue, value, _bCheckError);
        _outValue             = static_cast<T>(value);
        return bSucceeded;
    }

    // ===========================================
    //  ReadTo Custom
    // ===========================================

    template<JsonReaderableByCustomT T>
    bool ReadTo_(
        yyjson_val* _pValue,
        T&          _outValue,
        const bool  _bCheckError)
    {
        JUG_ASSERT(_pValue, "Value must not be null - caller assumed a wrong JSON layout");

        // detect whether the nested deserialize raised an error via the generation counter.
        const int lastGen = m_errorGen;

        // custom deserialize
        m_pPendingValue = _pValue;
        if constexpr (JsonReaderableByFactoryT<T>)
        {
            _outValue = CallFactory_<T>();
        }
        else if constexpr (JsonReaderableByMethodT<T>)
        {
            _outValue.Deserialize(*this);
        }
        else
        {
            Deserialize(*this, _outValue);
        }
        m_pPendingValue = nullptr;

        return m_errorGen == lastGen;
    }

    // ===========================================
    //  Read
    // ===========================================

    template<typename T>
    [[nodiscard]] T Read_(
        yyjson_val* _pValue,
        bool        _bCheckError)
    {
        JUG_ASSERT(_pValue, "Value must not be null - caller assumed a wrong JSON layout");

        if constexpr (std::is_default_constructible_v<T>)
        {
            T value;
            ReadTo_(_pValue, value, _bCheckError);
            return value;
        }
        else
        {
            m_pPendingValue = _pValue;
            T value         = CallFactory_<T>();
            m_pPendingValue = nullptr;
            return value;
        }
    }

    template<JsonReaderableByFactoryT T>
    [[nodiscard]] T CallFactory_()
    {
        JUG_ASSERT(m_pPendingValue, "Pending value must not be null - caller assumed a wrong JSON layout");

        if constexpr (JsonReaderableByFactoryMethodT<T>)
        {
            return T::Deserialize(*this);
        }
        else
        {
            return Deserialize(*this, kJsonTypeTag<T>);
        }
    }

    yyjson_doc*      m_pDoc          = nullptr;
    yyjson_val*      m_pRoot         = nullptr;
    yyjson_val*      m_pPendingValue = nullptr;
    Vector<Frame>    m_frameStack    = {};
    eSerializerError m_error         = eSerializerError::None;
    std::string      m_errorMsg      = {};
    int              m_errorGen      = 0;
};

}   // namespace jug
