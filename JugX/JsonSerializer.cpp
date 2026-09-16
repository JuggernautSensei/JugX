#include "pch.h"
#include "JsonSerializer.h"

#include <yyjson.h>

#include "CoreLogger.h"
#include "Error.h"
#include "FileWriter.h"
#include "SerializeError.h"

namespace jug
{

namespace
{

    [[nodiscard]] void* DefaultMalloc(
        void*,
        const size_t _size)
    {
        return std::malloc(_size);
    }

    [[nodiscard]] void* DefaultRealloc(
        void*,
        void* _ptr,
        const size_t,
        const size_t _size)
    {
        return std::realloc(_ptr, _size);
    }

    void DefaultFree(
        void*,
        void* _ptr)
    {
        std::free(_ptr);
    }

    constexpr yyjson_alc kDefaultAlc = {
        DefaultMalloc,
        DefaultRealloc,
        DefaultFree,
        nullptr
    };

}   // namespace

// ==========================================================
//  JsonWriter
// ==========================================================

void JsonWriter::SetObject()
{
    SetValue_(yyjson_mut_obj(m_pDoc));
}

void JsonWriter::SetArray()
{
    SetValue_(yyjson_mut_arr(m_pDoc));
}

JsonWriter JsonWriter::BeginObjectField(
    const StringView _key) const
{
    JUG_ASSERT(IsObject(), "BeginObjectField requires an object value - call SetObject() first");

    yyjson_mut_val* pField = yyjson_mut_obj(m_pDoc);
    AddField_(_key, pField);
    return JsonWriter { m_pDoc, pField };
}

JsonWriter JsonWriter::BeginArrayField(
    const StringView _key) const
{
    JUG_ASSERT(IsObject(), "BeginArrayField requires an object value - call SetObject() first");

    yyjson_mut_val* pField = yyjson_mut_arr(m_pDoc);
    AddField_(_key, pField);
    return JsonWriter { m_pDoc, pField };
}

JsonWriter JsonWriter::PushBackObject() const
{
    JUG_ASSERT(IsArray(), "PushBackObject requires an array value - call SetArray() first");

    yyjson_mut_val* pElement = yyjson_mut_obj(m_pDoc);
    AddElement_(pElement);
    return JsonWriter { m_pDoc, pElement };
}

JsonWriter JsonWriter::PushBackArray() const
{
    JUG_ASSERT(IsArray(), "PushBackArray requires an array value - call SetArray() first");

    yyjson_mut_val* pElement = yyjson_mut_arr(m_pDoc);
    AddElement_(pElement);
    return JsonWriter { m_pDoc, pElement };
}

bool JsonWriter::IsObject() const
{
    return m_pValue && yyjson_mut_is_obj(m_pValue);
}

bool JsonWriter::IsArray() const
{
    return m_pValue && yyjson_mut_is_arr(m_pValue);
}

JsonWriter::JsonWriter(
    yyjson_mut_doc* const _pDoc,
    yyjson_mut_val* const _pValue)
    : m_pDoc(_pDoc)
    , m_pValue(_pValue)
{
}

void JsonWriter::SetValue_(
    yyjson_mut_val* const _pValue)
{
    JUG_ASSERT(m_pValue == nullptr, "this value has already been written - a value can only be written once");
    JUG_ASSERT(_pValue, "failed to allocate a JSON value");

    m_pValue = _pValue;

    // 최초로 만들어지는 값은 루트로 설정
    if (!yyjson_mut_doc_get_root(m_pDoc))
    {
        yyjson_mut_doc_set_root(m_pDoc, m_pValue);
    }
}

void JsonWriter::AddField_(
    const StringView      _key,
    yyjson_mut_val* const _pValue) const
{
    yyjson_mut_val* pKey = yyjson_mut_strncpy(m_pDoc, _key.data(), _key.size());
    JUG_ASSERT(pKey, "failed to allocate a key string");
    yyjson_mut_obj_add(m_pValue, pKey, _pValue);
}

void JsonWriter::AddElement_(
    yyjson_mut_val* const _pValue) const
{
    yyjson_mut_arr_add_val(m_pValue, _pValue);
}

void JsonWriter::Write_(std::nullptr_t)
{
    SetValue_(yyjson_mut_null(m_pDoc));
}

void JsonWriter::Write_(
    const bool _value)
{
    SetValue_(yyjson_mut_bool(m_pDoc, _value));
}

void JsonWriter::Write_(
    const int64_t _value)
{
    SetValue_(yyjson_mut_sint(m_pDoc, _value));
}

void JsonWriter::Write_(
    const uint64_t _value)
{
    SetValue_(yyjson_mut_uint(m_pDoc, _value));
}

void JsonWriter::Write_(
    const double _value)
{
    SetValue_(yyjson_mut_real(m_pDoc, _value));
}

void JsonWriter::Write_(
    const StringView _value)
{
    SetValue_(yyjson_mut_strncpy(m_pDoc, _value.data(), _value.size()));
}

void JsonWriter::Write_(
    const String& _value)
{
    SetValue_(yyjson_mut_strncpy(m_pDoc, _value.data(), _value.size()));
}

void JsonWriter::Write_(
    const char* const _value)
{
    SetValue_(yyjson_mut_strcpy(m_pDoc, _value));
}

// ==========================================================
//  JsonSerializer
// ==========================================================

JsonSerializer::JsonSerializer()
    : m_pDoc(yyjson_mut_doc_new(nullptr))
{
    JUG_ASSERT(m_pDoc, "JsonSerializer: failed to allocate a mutable document");
}

JsonSerializer::~JsonSerializer()
{
    if (m_pDoc)
    {
        yyjson_mut_doc_free(m_pDoc);
    }
}

JsonSerializer::JsonSerializer(
    JsonSerializer&& _other) noexcept
    : m_pDoc(std::exchange(_other.m_pDoc, nullptr))
{
}

JsonSerializer& JsonSerializer::operator=(
    JsonSerializer&& _other) noexcept
{
    if (this != &_other)
    {
        if (m_pDoc)
        {
            yyjson_mut_doc_free(m_pDoc);
        }
        m_pDoc = std::exchange(_other.m_pDoc, nullptr);
    }
    return *this;
}

JsonWriter JsonSerializer::GetWriter() const
{
    JUG_ASSERT(m_pDoc, "JsonSerializer: the document has been moved out");
    return JsonWriter { m_pDoc, yyjson_mut_doc_get_root(m_pDoc) };
}

Result<String> JsonSerializer::Save(
    const Flags<eJsonSaveOption> _flags) const
{
    size_t           len;
    yyjson_write_err err;
    char*            pText = yyjson_mut_write_opts(m_pDoc, _flags.GetFlags(), &kDefaultAlc, &len, &err);
    if (!pText)
    {
        JUG_CORE_LOG_ERROR("JsonSerializer: failed to serialize the document - error code: {}, message: {}", err.code, err.msg);
        return eSerializeError::OutputFailed;
    }

    String text { pText, len };
    kDefaultAlc.free(kDefaultAlc.ctx, pText);
    return text;
}

Error JsonSerializer::SaveToFile(
    const FilePath&              _path,
    const Flags<eJsonSaveOption> _flags) const
{
    Result<FileWriter> writer = FileWriter::Open(_path);
    JUG_RETURN_IF_ERROR(writer);

    if (!yyjson_mut_write_fp(writer->GetFile(), m_pDoc, _flags.GetFlags(), nullptr, nullptr))
    {
        JUG_CORE_LOG_ERROR("JsonSerializer: failed to serialize the document into the given file");
        return eSerializeError::OutputFailed;
    }
    return eSerializeError::None;
}

}   // namespace jug
