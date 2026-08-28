#include "JsonReader.h"

#include <cmath>
#include <cstdlib>
#include <utility>

#include "Config.h"
#include "FileIO.h"
#include "Vendor/yyjson/src/yyjson.h"

namespace jug
{

bool JsonReader::Frame::TryNext()
{
    if (index >= size)
    {
        return false;
    }

    ++index;
    if (index == size)
    {
        pKey   = nullptr;
        pValue = nullptr;
    }
    else
    {
        if (bObject)
        {
            pKey   = unsafe_yyjson_get_next(pValue);
            pValue = pKey + 1;
        }
        else
        {
            pValue = unsafe_yyjson_get_next(pValue);
        }
    }
    return true;
}

SerializerResult<JsonReader> JsonReader::LoadFromString(
    const std::string_view       _json,
    const Flags<eJsonReadOption> _options)
{
    JsonReader             reader = {};
    const eSerializerError error  = reader.InitFromString_(_json, _options);
    if (error != eSerializerError::None)
    {
        return Failed { error };
    }
    return SerializerResult { std::move(reader) };
}

SerializerResult<JsonReader> JsonReader::LoadFromFile(
    const std::filesystem::path& _path,
    const Flags<eJsonReadOption> _options)
{
    JsonReader             reader = {};
    const eSerializerError error  = reader.InitFromFile_(_path, _options);
    if (error != eSerializerError::None)
    {
        return Failed { error };
    }
    return SerializerResult { std::move(reader) };
}

JsonReader::~JsonReader()
{
    if (m_pDoc != nullptr)
    {
        yyjson_doc_free(m_pDoc);
        m_pDoc = nullptr;
    }
}

JsonReader::JsonReader(
    JsonReader&& _other) noexcept
    : m_pDoc(std::exchange(_other.m_pDoc, nullptr))
    , m_pRoot(std::exchange(_other.m_pRoot, nullptr))
    , m_pPendingValue(std::exchange(_other.m_pPendingValue, nullptr))
    , m_frameStack(std::move(_other.m_frameStack))
    , m_lastError(std::exchange(_other.m_lastError, eSerializerError::None))
{
}

JsonReader& JsonReader::operator=(
    JsonReader&& _other) noexcept
{
    if (this != &_other)
    {
        if (m_pDoc != nullptr)
        {
            yyjson_doc_free(m_pDoc);
        }

        m_pDoc          = std::exchange(_other.m_pDoc, nullptr);
        m_pRoot         = std::exchange(_other.m_pRoot, nullptr);
        m_pPendingValue = std::exchange(_other.m_pPendingValue, nullptr);
        m_frameStack    = std::move(_other.m_frameStack);
        m_lastError     = std::exchange(_other.m_lastError, eSerializerError::None);
    }
    return *this;
}

bool JsonReader::BeginObject()
{
    yyjson_val* pValue = GetValue_(true);
    if (!unsafe_yyjson_is_obj(pValue))
    {
        m_lastError = eSerializerError::TypeMismatch;
        return false;
    }

    PushFrame_(pValue, true);
    return true;
}

bool JsonReader::BeginObject(
    const std::string_view _key)
{
    yyjson_val* pValue = FindFieldOrNull_(_key, true);
    if (!pValue)
    {
        m_lastError = eSerializerError::MissingField;
        return false;
    }

    if (!unsafe_yyjson_is_obj(pValue))
    {
        m_lastError = eSerializerError::TypeMismatch;
        return false;
    }

    PushFrame_(pValue, true);
    return true;
}

void JsonReader::EndObject()
{
    JUG_ASSERT(!m_frameStack.empty() && m_frameStack.back().bObject, "EndObject: no frame to pop - call BeginObject first");
    m_frameStack.pop_back();
}

bool JsonReader::HasField(
    const std::string_view _key) const
{
    JUG_ASSERT(!m_frameStack.empty(), "HasField: no frame to search - call BeginObject first");
    const Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "HasField: requires an object frame - call BeginObject first");
    return yyjson_obj_getn(frame.pContainer, _key.data(), _key.size()) != nullptr;
}

std::string_view JsonReader::GetKey() const
{
    JUG_ASSERT(!m_frameStack.empty(), "GetKey: no frame to get key - call BeginObject first");
    const Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "GetKey: requires an object frame - call BeginObject first");
    JUG_ASSERT(frame.pKey, "GetKey: no key to get - call BeginObject first");
    return { unsafe_yyjson_get_str(frame.pKey), unsafe_yyjson_get_len(frame.pKey) };
}

bool JsonReader::BeginArray()
{
    yyjson_val* pValue = GetValue_(true);
    if (!unsafe_yyjson_is_arr(pValue))
    {
        m_lastError = eSerializerError::TypeMismatch;
        return false;
    }
    PushFrame_(pValue, false);
    return true;
}

bool JsonReader::BeginArray(
    const std::string_view _key)
{
    yyjson_val* pValue = FindFieldOrNull_(_key, true);
    if (!pValue)
    {
        m_lastError = eSerializerError::MissingField;
        return false;
    }

    if (!unsafe_yyjson_is_arr(pValue))
    {
        m_lastError = eSerializerError::TypeMismatch;
        return false;
    }

    PushFrame_(pValue, false);
    return true;
}

void JsonReader::EndArray()
{
    JUG_ASSERT(!m_frameStack.empty() && !m_frameStack.back().bObject, "EndArray: no frame to pop - call BeginArray first");
    m_frameStack.pop_back();
}

void JsonReader::Next()
{
    JUG_ASSERT(!m_frameStack.empty(), "Next: no frame to advance - call BeginObject/BeginArray first");
    Frame& frame = m_frameStack.back();
    if (!frame.TryNext())
    {
        JUG_ASSERT(false, "Next: no more elements to read - call HasNext first");
    }
}

bool JsonReader::HasNext() const
{
    JUG_ASSERT(!m_frameStack.empty(), "HasNext: no frame to check - call BeginObject/BeginArray first");
    const Frame& frame = m_frameStack.back();
    return frame.index < frame.size;
}

size_t JsonReader::GetSize() const
{
    JUG_ASSERT(!m_frameStack.empty(), "GetSize: no frame to get size - call BeginObject/BeginArray first");
    return m_frameStack.back().size;
}

bool JsonReader::IsNull()
{
    return yyjson_get_type(GetValue_(false)) == YYJSON_TYPE_NULL;
}

bool JsonReader::IsArray()
{
    return yyjson_get_type(GetValue_(false)) == YYJSON_TYPE_ARR;
}

bool JsonReader::IsObject()
{
    return yyjson_get_type(GetValue_(false)) == YYJSON_TYPE_OBJ;
}

bool JsonReader::IsContainer()
{
    const yyjson_type type = yyjson_get_type(GetValue_(false));
    return type == YYJSON_TYPE_ARR || type == YYJSON_TYPE_OBJ;
}

bool JsonReader::IsValue()
{
    return !IsContainer();
}

bool JsonReader::HasError() const
{
    return m_lastError != eSerializerError::None;
}

eSerializerError JsonReader::GetLastError() const
{
    return m_lastError;
}

eSerializerError JsonReader::InitFromString_(
    const std::string_view       _json,
    const Flags<eJsonReadOption> _options)
{
    JUG_ASSERT(m_pDoc == nullptr && m_pRoot == nullptr, "InitFromString_: JsonReader is already initialized");

    yyjson_doc* pDoc = yyjson_read(_json.data(), _json.size(), _options.GetFlags());
    if (pDoc == nullptr)
    {
        return eSerializerError::ParseFailed;
    }

    yyjson_val* pRoot = yyjson_doc_get_root(pDoc);
    if (pRoot == nullptr)
    {
        yyjson_doc_free(pDoc);
        return eSerializerError::ParseFailed;
    }

    m_pDoc  = pDoc;
    m_pRoot = pRoot;
    return eSerializerError::None;
}

eSerializerError JsonReader::InitFromFile_(
    const std::filesystem::path& _path,
    const Flags<eJsonReadOption> _options)
{
    JUG_ASSERT(m_pDoc == nullptr && m_pRoot == nullptr, "InitFromFile_: JsonReader is already initialized");

    // open file
    FileIOResult<FileReader> file = FileReader::Open(_path);
    if (!file)
    {
        return eSerializerError::FileError;
    }

    yyjson_doc* pDoc = yyjson_read_fp(file->GetFile(), _options.GetFlags(), nullptr, nullptr);
    if (!pDoc)
    {
        return eSerializerError::ParseFailed;
    }

    yyjson_val* pRoot = yyjson_doc_get_root(pDoc);
    if (!pRoot)
    {
        yyjson_doc_free(pDoc);
        return eSerializerError::ParseFailed;
    }

    m_pDoc  = pDoc;
    m_pRoot = pRoot;
    return eSerializerError::None;
}

yyjson_val* JsonReader::GetValue_(
    const bool _bNext)
{
    if (m_pPendingValue)
    {
        return _bNext ? std::exchange(m_pPendingValue, nullptr) : m_pPendingValue;
    }

    if (m_frameStack.empty())
    {
        return m_pRoot;
    }

    Frame&      frame  = m_frameStack.back();
    yyjson_val* pValue = frame.pValue;
    JUG_ASSERT(pValue, "GetValue_: no value to read");
    if (_bNext)
    {
        if (!frame.TryNext())
        {
            JUG_ASSERT(false, "GetValue_: no more elements to read");
        }
    }
    return pValue;
}

yyjson_val* JsonReader::FindFieldOrNull_(
    const std::string_view _key,
    const bool             _bCheckError)
{
    JUG_ASSERT(!m_frameStack.empty(), "FindFieldOrNull_: no frame to search - call BeginObject first");
    Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "FindFieldOrNull_: requires an object frame - call BeginObject first");
    yyjson_val* pValue = yyjson_obj_getn(frame.pContainer, _key.data(), _key.size());
    if (!pValue && _bCheckError)
    {
        m_lastError = eSerializerError::MissingField;
    }
    return pValue;
}

void JsonReader::PushFrame_(
    yyjson_val* _pValue,
    const bool  _bObject)
{
    JUG_ASSERT(_pValue, "PushFrame_: value must not be null - caller assumed a wrong JSON layout");

    Frame frame   = {};
    frame.bObject = _bObject;

    if (_bObject)
    {
        yyjson_obj_iter iter;
        yyjson_obj_iter_init(_pValue, &iter);
        frame.pContainer = _pValue;
        frame.pKey       = iter.cur;
        frame.pValue     = frame.pKey + 1;
        frame.index      = iter.idx;
        frame.size       = iter.max;
    }
    else
    {
        yyjson_arr_iter iter;
        yyjson_arr_iter_init(_pValue, &iter);
        frame.pContainer = _pValue;
        frame.pKey       = nullptr;
        frame.pValue     = iter.cur;
        frame.index      = iter.idx;
        frame.size       = iter.max;
    }

    m_frameStack.push_back(frame);
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    bool&             _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "ReadTo_: value must not be null");

    if (!unsafe_yyjson_is_bool(_pValue))
    {
        if (_bCheckError)
        {
            m_lastError = eSerializerError::TypeMismatch;
        }
        _outValue = false;
        return false;
    }

    _outValue = unsafe_yyjson_get_bool(_pValue);
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    int64_t&          _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "ReadTo_: value must not be null");

    if (!unsafe_yyjson_is_int(_pValue))
    {
        if (_bCheckError)
        {
            m_lastError = eSerializerError::TypeMismatch;
        }
        _outValue = 0;
        return false;
    }

    _outValue = unsafe_yyjson_get_sint(_pValue);
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    uint64_t&         _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "ReadTo_: value must not be null");

    if (!unsafe_yyjson_is_int(_pValue))
    {
        if (_bCheckError)
        {
            m_lastError = eSerializerError::TypeMismatch;
        }
        _outValue = 0;
        return false;
    }

    _outValue = unsafe_yyjson_get_uint(_pValue);
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    double&           _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "ReadTo_: value must not be null");

    if (!unsafe_yyjson_is_num(_pValue))
    {
        if (_bCheckError)
        {
            m_lastError = eSerializerError::TypeMismatch;
        }
        _outValue = 0.;
        return false;
    }

    _outValue = unsafe_yyjson_get_num(_pValue);
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    std::string_view& _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "ReadTo_: value must not be null");

    if (!unsafe_yyjson_is_str(_pValue))
    {
        if (_bCheckError)
        {
            m_lastError = eSerializerError::TypeMismatch;
        }
        _outValue = {};
        return false;
    }

    _outValue = { unsafe_yyjson_get_str(_pValue), unsafe_yyjson_get_len(_pValue) };
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    std::string&      _outValue,
    const bool        _bCheckError)
{
    std::string_view value;
    if (!ReadTo_(_pValue, value, _bCheckError))
    {
        _outValue = {};
        return false;
    }

    _outValue = std::string { value };
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    const char*&      _outValue,
    const bool        _bCheckError)
{
    std::string_view value;
    if (!ReadTo_(_pValue, value, _bCheckError))
    {
        _outValue = nullptr;
        return false;
    }

    _outValue = value.data();
    return true;
}

}   // namespace jug
