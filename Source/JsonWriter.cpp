#include "JsonWriter.h"

#include <cstdlib>
#include <utility>

#include "Config.h"
#include "FileReaderWriter.h"
#include "MemoryView.h"
#include "Vendor/yyjson/src/yyjson.h"

namespace jug
{

JsonWriter::JsonWriter()
    : m_pDoc(yyjson_mut_doc_new(nullptr))
{
    JUG_ASSERT(m_pDoc, "JsonWriter: failed to allocate a mutable document");
}

JsonWriter::~JsonWriter()
{
    if (m_pDoc != nullptr)
    {
        yyjson_mut_doc_free(m_pDoc);
        m_pDoc = nullptr;
    }
}

JsonWriter::JsonWriter(
    JsonWriter&& _other) noexcept
    : m_pDoc(std::exchange(_other.m_pDoc, nullptr))
    , m_pPendingKey(std::exchange(_other.m_pPendingKey, nullptr))
    , m_frameStack(std::move(_other.m_frameStack))
{
}

JsonWriter& JsonWriter::operator=(
    JsonWriter&& _other) noexcept
{
    if (this != &_other)
    {
        if (m_pDoc != nullptr)
        {
            yyjson_mut_doc_free(m_pDoc);
        }

        m_pDoc        = std::exchange(_other.m_pDoc, nullptr);
        m_pPendingKey = std::exchange(_other.m_pPendingKey, nullptr);
        m_frameStack  = std::move(_other.m_frameStack);
    }
    return *this;
}

void JsonWriter::BeginObject()
{
    PushFrame_(true);
}

void JsonWriter::BeginObject(
    const StringView _key)
{
    WriteKey(_key);
    PushFrame_(true);
}

void JsonWriter::EndObject()
{
    JUG_ASSERT(!m_frameStack.empty() && m_frameStack.back().bObject, "EndObject: no frame to pop - call BeginObject first");
    JUG_ASSERT(!m_pPendingKey, "EndObject: pending key has no value");
    m_frameStack.pop_back();
}

void JsonWriter::WriteKey(
    const StringView _key)
{
    JUG_ASSERT(!m_frameStack.empty() && m_frameStack.back().bObject, "WriteKey: requires an object frame - call BeginObject first");
    JUG_ASSERT(!m_pPendingKey, "WriteKey: previous key has no value yet");
    m_pPendingKey = yyjson_mut_strncpy(m_pDoc, _key.data(), _key.size());
    JUG_ASSERT(m_pPendingKey, "WriteKey: failed to allocate a key string");
}

void JsonWriter::BeginArray()
{
    PushFrame_(false);
}

void JsonWriter::BeginArray(
    const StringView _key)
{
    WriteKey(_key);
    PushFrame_(false);
}

void JsonWriter::EndArray()
{
    JUG_ASSERT(!m_frameStack.empty() && !m_frameStack.back().bObject, "EndArray: no frame to pop - call BeginArray first");
    m_frameStack.pop_back();
}

void JsonWriter::AttachValue_(
    yyjson_mut_val* _pValue)
{
    JUG_ASSERT(_pValue, "AttachValue_: value must not be null - caller assumed a wrong JSON layout");

    if (m_frameStack.empty())
    {
        JUG_ASSERT(!yyjson_mut_doc_get_root(m_pDoc), "AttachValue_: root value can only be written once");
        JUG_ASSERT(!m_pPendingKey, "AttachValue_: key is not allowed at root scope");
        yyjson_mut_doc_set_root(m_pDoc, _pValue);
        return;
    }

    const Frame& frame = m_frameStack.back();
    if (frame.bObject)
    {
        JUG_ASSERT(m_pPendingKey, "AttachValue_: object frame requires a key - use WriteKey/WriteField\n");
        yyjson_mut_obj_add(frame.pContainer, std::exchange(m_pPendingKey, nullptr), _pValue);
        return;
    }

    JUG_ASSERT(!m_pPendingKey, "AttachValue_: key is not allowed in an array frame");
    yyjson_mut_arr_add_val(frame.pContainer, _pValue);
}

void JsonWriter::PushFrame_(
    const bool _bObject)
{
    yyjson_mut_val* pContainer = _bObject ? yyjson_mut_obj(m_pDoc) : yyjson_mut_arr(m_pDoc);
    AttachValue_(pContainer);

    Frame frame      = {};
    frame.bObject    = _bObject;
    frame.pContainer = pContainer;
    m_frameStack.push_back(frame);
}

void JsonWriter::Write_(
    const std::nullptr_t)
{
    AttachValue_(yyjson_mut_null(m_pDoc));
}

void JsonWriter::Write_(
    const bool _value)
{
    AttachValue_(yyjson_mut_bool(m_pDoc, _value));
}

void JsonWriter::Write_(
    const int64_t _value)
{
    AttachValue_(yyjson_mut_sint(m_pDoc, _value));
}

void JsonWriter::Write_(
    const uint64_t _value)
{
    AttachValue_(yyjson_mut_uint(m_pDoc, _value));
}

void JsonWriter::Write_(
    const double _value)
{
    AttachValue_(yyjson_mut_real(m_pDoc, _value));
}

void JsonWriter::Write_(
    const StringView _value)
{
    AttachValue_(yyjson_mut_strncpy(m_pDoc, _value.data(), _value.size()));
}

void JsonWriter::Write_(
    const char* _value)
{
    AttachValue_(yyjson_mut_strcpy(m_pDoc, _value));
}

SerializerResult<String> JsonWriter::SaveToString(
    const Flags<eJsonWriteOption> _options) const
{
    JUG_ASSERT(m_frameStack.empty(), "SaveToString: unclosed BeginObject/BeginArray");
    JUG_ASSERT(!m_pPendingKey, "SaveToString: pending key has no value");

    size_t len;
    char*  pText = yyjson_mut_write_opts(m_pDoc, _options.GetFlags(), nullptr, &len, nullptr);
    if (!pText)
    {
        return Failed { eSerializerError::OutputFailed };
    }

    String text(pText, len);
    std::free(pText);
    return text;
}

eSerializerError JsonWriter::SaveToFile(
    const FilePath&  _path,
    const Flags<eJsonWriteOption> _options) const
{
    JUG_ASSERT(m_frameStack.empty(), "SaveToString: unclosed BeginObject/BeginArray");
    JUG_ASSERT(!m_pPendingKey, "SaveToString: pending key has no value");

    FileResult<FileWriter> writer = FileWriter::Open(_path);
    if (!writer)
    {
        return eSerializerError::FileError;
    }

    if (!yyjson_mut_write_fp(writer->GetFile(), m_pDoc, _options.GetFlags(), nullptr, nullptr))
    {
        return eSerializerError::OutputFailed;
    }
    return eSerializerError::None;
}

}   // namespace jug
