#include "JsonDeserializer.h"

#include <compare>
#include <cstddef>
#include <type_traits>
#include <cstdint>
#include <utility>

#include "Assertion.h"
#include "CoreLogger.h"
#include "EnumFlags.h"
#include "FileReader.h"
#include "Typedef.h"
#include "Result.h"
#include "SerializeError.h"
#include "Vendor/yyjson/src/yyjson.h"

namespace jug
{

namespace
{

    [[nodiscard]] StringView JsonTypeString_(
        const yyjson_val* const _pValue)
    {
        switch (yyjson_get_type(_pValue))
        {
            case YYJSON_TYPE_NONE: return "none";
            case YYJSON_TYPE_NULL: return "null";
            case YYJSON_TYPE_BOOL: return "bool";
            case YYJSON_TYPE_NUM: return "number";
            case YYJSON_TYPE_STR: return "string";
            case YYJSON_TYPE_ARR: return "array";
            case YYJSON_TYPE_OBJ: return "object";
            default: return "unknown";
        }
    }

    void LogTypeMismatch_(
        const yyjson_val* const _pValue,
        const StringView        _expected)
    {
        JUG_CORE_LOG_ERROR("JsonReader: expected a JSON {}, but the value is a {}", _expected, JsonTypeString_(_pValue));
    }

}   // namespace

// ==========================================================
//  JsonReader::ArrayIterator
// ==========================================================

JsonReader JsonReader::ArrayIterator::operator*() const
{
    JUG_ASSERT(m_pValue, "dereferencing an end iterator");
    return JsonReader { m_pValue };
}

JsonReader::ArrayIterator& JsonReader::ArrayIterator::operator++()
{
    JUG_ASSERT(m_pValue, "incrementing an end iterator");
    --m_remain;
    m_pValue = (m_remain > 0) ? unsafe_yyjson_get_next(m_pValue) : nullptr;
    return *this;
}

JsonReader::ArrayIterator JsonReader::ArrayIterator::operator++(
    const int)
{
    ArrayIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool JsonReader::ArrayIterator::operator==(
    const ArrayIterator& _other) const
{
    return m_pValue == _other.m_pValue;
}

std::strong_ordering JsonReader::ArrayIterator::operator<=>(
    const ArrayIterator& _other) const
{
    return _other.m_remain <=> m_remain;
}

JsonReader::ArrayIterator JsonReader::ArrayRange::begin() const
{
    return m_iter;
}

JsonReader::ArrayIterator JsonReader::ArrayRange::end() const
{
    return ArrayIterator {};
}

JsonReader::ArrayRange::ArrayRange(
    const ArrayIterator _iter)
    : m_iter(_iter)
{
}

JsonReader::ObjectIterator JsonReader::ObjectRange::begin() const
{
    return m_iter;
}

JsonReader::ObjectIterator JsonReader::ObjectRange::end() const
{
    return ObjectIterator {};
}

JsonReader::ObjectRange::ObjectRange(const ObjectIterator _iter)
    : m_iter(_iter)
{
}

// ===========================================
//  JsonReader
// ===========================================

Result<JsonReader> JsonReader::FindField(
    const StringView _key) const
{
    if (!IsObject())
    {
        LogTypeMismatch_(m_pValue, "object");
        return eSerializeError::TypeMismatch;
    }

    yyjson_val* pField = yyjson_obj_getn(m_pValue, _key.data(), _key.size());
    if (!pField)
    {
        JUG_CORE_LOG_ERROR("JsonReader: field '{}' not found", _key);
        return eSerializeError::MissingField;
    }

    return JsonReader { pField };
}

JsonReader::ArrayIterator JsonReader::BeginArray() const
{
    if (!IsArray())
    {
        LogTypeMismatch_(m_pValue, "array");
        return ArrayIterator {};
    }

    yyjson_arr_iter iter;
    yyjson_arr_iter_init(m_pValue, &iter);
    if (iter.max == 0)
    {
        return ArrayIterator {};
    }
    return ArrayIterator { iter.cur, iter.max };
}

JsonReader::ArrayIterator JsonReader::EndArray() const
{
    return ArrayIterator {};
}

JsonReader::ArrayRange JsonReader::GetArray() const
{
    return ArrayRange { BeginArray() };
}

JsonReader::ObjectIterator JsonReader::BeginObject() const
{
    if (!IsObject())
    {
        LogTypeMismatch_(m_pValue, "object");
        return ObjectIterator {};
    }

    yyjson_obj_iter iter;
    yyjson_obj_iter_init(m_pValue, &iter);
    if (iter.max == 0)
    {
        return ObjectIterator {};
    }
    return ObjectIterator { iter.cur, iter.max };
}

JsonReader::ObjectIterator JsonReader::EndObject() const
{
    return ObjectIterator {};
}

JsonReader::ObjectRange JsonReader::GetObject() const
{
    return ObjectRange { BeginObject() };
}

bool JsonReader::HasField(
    const StringView _key) const
{
    JUG_ASSERT(IsObject(), "HasField requires an object value");
    return yyjson_obj_getn(m_pValue, _key.data(), _key.size());
}

size_t JsonReader::GetSize() const
{
    JUG_ASSERT(IsContainer(), "GetSize requires an array or object value");
    return unsafe_yyjson_get_len(m_pValue);
}

bool JsonReader::IsNull() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_NULL;
}

bool JsonReader::IsBool() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_BOOL;
}

bool JsonReader::IsNumber() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_NUM;
}

bool JsonReader::IsString() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_STR;
}

bool JsonReader::IsArray() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_ARR;
}

bool JsonReader::IsObject() const
{
    return yyjson_get_type(m_pValue) == YYJSON_TYPE_OBJ;
}

bool JsonReader::IsContainer() const
{
    const yyjson_type type = yyjson_get_type(m_pValue);
    return type == YYJSON_TYPE_ARR || type == YYJSON_TYPE_OBJ;
}

JsonReader::JsonReader(
    const yyjson_val* const _pValue)
    : m_pValue(_pValue)
{
}

Result<bool> JsonReader::Read_(
    std::type_identity<bool>) const
{
    if (!IsBool())
    {
        LogTypeMismatch_(m_pValue, "bool");
        return eSerializeError::TypeMismatch;
    }
    return unsafe_yyjson_get_bool(m_pValue);
}

Result<int64_t> JsonReader::Read_(
    std::type_identity<int64_t>) const
{
    if (!IsNumber() || !unsafe_yyjson_is_int(m_pValue))
    {
        LogTypeMismatch_(m_pValue, "integer");
        return eSerializeError::TypeMismatch;
    }
    return unsafe_yyjson_get_sint(m_pValue);
}

Result<uint64_t> JsonReader::Read_(
    std::type_identity<uint64_t>) const
{
    if (!IsNumber() || !unsafe_yyjson_is_int(m_pValue))
    {
        LogTypeMismatch_(m_pValue, "integer");
        return eSerializeError::TypeMismatch;
    }
    return unsafe_yyjson_get_uint(m_pValue);
}

Result<double> JsonReader::Read_(
    std::type_identity<double>) const
{
    if (!IsNumber())
    {
        LogTypeMismatch_(m_pValue, "number");
        return eSerializeError::TypeMismatch;
    }
    return unsafe_yyjson_get_num(m_pValue);
}

Result<StringView> JsonReader::Read_(
    std::type_identity<StringView>) const
{
    if (!IsString())
    {
        LogTypeMismatch_(m_pValue, "string");
        return eSerializeError::TypeMismatch;
    }
    return StringView { unsafe_yyjson_get_str(m_pValue), unsafe_yyjson_get_len(m_pValue) };
}

Result<String> JsonReader::Read_(
    std::type_identity<String>) const
{
    if (!IsString())
    {
        LogTypeMismatch_(m_pValue, "string");
        return eSerializeError::TypeMismatch;
    }
    return String { unsafe_yyjson_get_str(m_pValue), unsafe_yyjson_get_len(m_pValue) };
}

Result<const char*> JsonReader::Read_(
    std::type_identity<const char*>) const
{
    if (!IsString())
    {
        LogTypeMismatch_(m_pValue, "string");
        return eSerializeError::TypeMismatch;
    }
    return unsafe_yyjson_get_str(m_pValue);
}

// ==========================================================
//  JsonReader::ObjectIterator
// ==========================================================

std::pair<StringView, JsonReader> JsonReader::ObjectIterator::operator*() const
{
    JUG_ASSERT(m_pKey, "dereferencing an end iterator");
    return {
        StringView { unsafe_yyjson_get_str(m_pKey), unsafe_yyjson_get_len(m_pKey) },
        JsonReader { m_pKey + 1 }
    };
}

JsonReader::ObjectIterator& JsonReader::ObjectIterator::operator++()
{
    JUG_ASSERT(m_pKey, "incrementing an end iterator");
    --m_remain;
    m_pKey = (m_remain > 0) ? unsafe_yyjson_get_next(m_pKey + 1) : nullptr;
    return *this;
}

JsonReader::ObjectIterator JsonReader::ObjectIterator::operator++(
    const int)
{
    ObjectIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool JsonReader::ObjectIterator::operator==(
    const ObjectIterator& _other) const
{
    return m_pKey == _other.m_pKey;
}

std::strong_ordering JsonReader::ObjectIterator::operator<=>(
    const ObjectIterator& _other) const
{
    return _other.m_remain <=> m_remain;
}

// ==========================================================
//  JsonDeserializer
// ==========================================================

Result<JsonDeserializer> JsonDeserializer::Load(
    const StringView             _json,
    const Flags<eJsonLoadOption> _flags)
{
    yyjson_doc* pDoc = yyjson_read(_json.data(), _json.size(), _flags.GetFlags());
    if (!pDoc)
    {
        JUG_CORE_LOG_ERROR("JsonDeserializer: failed to parse the given JSON string");
        return eSerializeError::ParseFailed;
    }

    if (!yyjson_doc_get_root(pDoc))
    {
        yyjson_doc_free(pDoc);
        JUG_CORE_LOG_ERROR("JsonDeserializer: the parsed document has no root value");
        return eSerializeError::ParseFailed;
    }

    return JsonDeserializer { pDoc };
}

Result<JsonDeserializer> JsonDeserializer::LoadFromFile(
    const FilePath&              _path,
    const Flags<eJsonLoadOption> _flags)
{
    Result<FileReader> reader = FileReader::Open(_path);
    JUG_RETURN_IF_ERROR(reader);

    yyjson_doc* pDoc = yyjson_read_fp(reader->GetFile(), _flags.GetFlags(), nullptr, nullptr);
    if (!pDoc)
    {
        JUG_CORE_LOG_ERROR("JsonDeserializer: failed to parse the given JSON file");
        return eSerializeError::ParseFailed;
    }

    if (!yyjson_doc_get_root(pDoc))
    {
        yyjson_doc_free(pDoc);
        JUG_CORE_LOG_ERROR("JsonDeserializer: the parsed document has no root value");
        return eSerializeError::ParseFailed;
    }

    return JsonDeserializer { pDoc };
}

JsonDeserializer::~JsonDeserializer()
{
    if (m_pDoc)
    {
        yyjson_doc_free(m_pDoc);
    }
}

JsonDeserializer::JsonDeserializer(
    JsonDeserializer&& _other) noexcept
    : m_pDoc(std::exchange(_other.m_pDoc, nullptr))
{
}

JsonDeserializer& JsonDeserializer::operator=(
    JsonDeserializer&& _other) noexcept
{
    if (this != &_other)
    {
        if (m_pDoc)
        {
            yyjson_doc_free(m_pDoc);
        }
        m_pDoc = std::exchange(_other.m_pDoc, nullptr);
    }
    return *this;
}

JsonReader JsonDeserializer::GetReader() const
{
    JUG_ASSERT(m_pDoc, "JsonDeserializer: the document has been moved out");
    return JsonReader { yyjson_doc_get_root(m_pDoc) };
}

JsonDeserializer::JsonDeserializer(
    yyjson_doc* const _pDoc)
    : m_pDoc(_pDoc)
{
}

}   // namespace jug
