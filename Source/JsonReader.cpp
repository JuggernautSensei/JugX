#include "JsonReader.h"

#include <cmath>
#include <cstdlib>
#include <format>
#include <utility>

#include "Config.h"
#include "FileReaderWriter.h"
#include "Vendor/yyjson/src/yyjson.h"

namespace jug
{

namespace
{

    [[nodiscard]] std::string_view JsonTypeString_(
        const yyjson_val* _pValue)
    {
        JUG_ASSERT(_pValue != nullptr, "_pValue is null");
        const yyjson_type type = unsafe_yyjson_get_type(_pValue);
        switch (type)
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

}   // namespace

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
    const StringView             _json,
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
    const FilePath&              _path,
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
    , m_error(std::exchange(_other.m_error, eSerializerError::None))
    , m_errorMsg(std::move(_other.m_errorMsg))
    , m_errorGen(std::exchange(_other.m_errorGen, 0))
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
        m_error         = std::exchange(_other.m_error, eSerializerError::None);
        m_errorMsg      = std::move(_other.m_errorMsg);
        m_errorGen      = std::exchange(_other.m_errorGen, 0);
    }
    return *this;
}

bool JsonReader::BeginObject()
{
    yyjson_val* pValue = GetValue_(true);
    if (!unsafe_yyjson_is_obj(pValue))
    {
        SetError_(eSerializerError::TypeMismatch, std::format("Current value is not a JSON object. It is a {}", JsonTypeString_(pValue)));
        return false;
    }

    PushFrame_(pValue, true);
    return true;
}

bool JsonReader::BeginObject(
    const StringView _key)
{
    yyjson_val* pValue = FindFieldOrNull_(_key, true);
    if (!pValue)
    {
        SetError_(eSerializerError::MissingField, std::format("Field '{}' not found", _key));
        return false;
    }

    if (!unsafe_yyjson_is_obj(pValue))
    {
        SetError_(eSerializerError::TypeMismatch, std::format("Field '{}' is not a JSON object. It is a {}", _key, JsonTypeString_(pValue)));
        return false;
    }

    PushFrame_(pValue, true);
    return true;
}

void JsonReader::EndObject()
{
    JUG_ASSERT(!m_frameStack.empty() && m_frameStack.back().bObject, "No frame to pop - call BeginObject first");
    m_frameStack.pop_back();
}

bool JsonReader::HasField(
    const StringView _key) const
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to search - call BeginObject first");
    const Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "Requires an object frame - call BeginObject first");
    return yyjson_obj_getn(frame.pContainer, _key.data(), _key.size()) != nullptr;
}

StringView JsonReader::GetKey() const
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to get key - call BeginObject first");
    const Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "Requires an object frame - call BeginObject first");
    JUG_ASSERT(frame.pKey, "No key to get - call BeginObject first");
    return { unsafe_yyjson_get_str(frame.pKey), unsafe_yyjson_get_len(frame.pKey) };
}

bool JsonReader::BeginArray()
{
    yyjson_val* pValue = GetValue_(true);
    if (!unsafe_yyjson_is_arr(pValue))
    {
        SetError_(eSerializerError::TypeMismatch, std::format("Current value is not a JSON array. It is a {}", JsonTypeString_(pValue)));
        return false;
    }
    PushFrame_(pValue, false);
    return true;
}

bool JsonReader::BeginArray(
    const StringView _key)
{
    yyjson_val* pValue = FindFieldOrNull_(_key, true);
    if (!pValue)
    {
        SetError_(eSerializerError::MissingField, std::format("Field '{}' not found", _key));
        return false;
    }

    if (!unsafe_yyjson_is_arr(pValue))
    {
        SetError_(eSerializerError::TypeMismatch, std::format("Field '{}' is not a JSON array. It is a {}", _key, JsonTypeString_(pValue)));
        return false;
    }

    PushFrame_(pValue, false);
    return true;
}

void JsonReader::EndArray()
{
    JUG_ASSERT(!m_frameStack.empty() && !m_frameStack.back().bObject, "No frame to pop - call BeginArray first");
    m_frameStack.pop_back();
}

void JsonReader::Next()
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to advance - call BeginObject/BeginArray first");
    Frame& frame = m_frameStack.back();
    if (!frame.TryNext())
    {
        JUG_ASSERT(false, "No more elements to read - call HasNext first");
    }
}

bool JsonReader::HasNext() const
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to check - call BeginObject/BeginArray first");
    const Frame& frame = m_frameStack.back();
    return frame.index < frame.size;
}

size_t JsonReader::GetSize() const
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to get size - call BeginObject/BeginArray first");
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
    return m_error != eSerializerError::None;
}

eSerializerError JsonReader::GetLastError() const
{
    return m_error;
}

std::string_view JsonReader::GetLastErrorMsg() const
{
    return m_errorMsg;
}

eSerializerError JsonReader::InitFromString_(
    const StringView             _json,
    const Flags<eJsonReadOption> _options)
{
    JUG_ASSERT(m_pDoc == nullptr && m_pRoot == nullptr, "JsonReader is already initialized");

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
    const FilePath&              _path,
    const Flags<eJsonReadOption> _options)
{
    JUG_ASSERT(m_pDoc == nullptr && m_pRoot == nullptr, "JsonReader is already initialized");

    // open file
    FileResult<FileReader> file = FileReader::Open(_path);
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
    JUG_ASSERT(pValue, "No value to read");
    if (_bNext)
    {
        if (!frame.TryNext())
        {
            JUG_ASSERT(false, "No more elements to read");
        }
    }
    return pValue;
}

yyjson_val* JsonReader::FindFieldOrNull_(
    const StringView _key,
    const bool       _bCheckError)
{
    JUG_ASSERT(!m_frameStack.empty(), "No frame to search - call BeginObject first");
    Frame& frame = m_frameStack.back();
    JUG_ASSERT(frame.bObject, "Requires an object frame - call BeginObject first");
    yyjson_val* pValue = yyjson_obj_getn(frame.pContainer, _key.data(), _key.size());
    if (!pValue && _bCheckError)
    {
        SetError_(eSerializerError::MissingField, std::format("Field '{}' not found", _key));
    }
    return pValue;
}

void JsonReader::PushFrame_(
    yyjson_val* _pValue,
    const bool  _bObject)
{
    JUG_ASSERT(_pValue, "Value must not be null - caller assumed a wrong JSON layout");

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

void JsonReader::SetError_(
    const eSerializerError _error,
    const std::string_view _msg)
{
    ++m_errorGen;
    m_error    = _error;
    m_errorMsg = _msg;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    bool&             _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "Value must not be null");

    if (!unsafe_yyjson_is_bool(_pValue))
    {
        if (_bCheckError)
        {
            SetError_(eSerializerError::TypeMismatch, std::format("Value is not a JSON boolean. It is a {}", JsonTypeString_(_pValue)));
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
    JUG_ASSERT(_pValue, "Value must not be null");

    if (!unsafe_yyjson_is_int(_pValue))
    {
        if (_bCheckError)
        {
            SetError_(eSerializerError::TypeMismatch, std::format("Value is not a JSON integer. It is a {}", JsonTypeString_(_pValue)));
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
    JUG_ASSERT(_pValue, "Value must not be null");

    if (!unsafe_yyjson_is_int(_pValue))
    {
        if (_bCheckError)
        {
            SetError_(eSerializerError::TypeMismatch, std::format("Value is not a JSON integer. It is a {}", JsonTypeString_(_pValue)));
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
    JUG_ASSERT(_pValue, "Value must not be null");

    if (!unsafe_yyjson_is_num(_pValue))
    {
        if (_bCheckError)
        {
            SetError_(eSerializerError::TypeMismatch, std::format("Value is not a JSON number. It is a {}", JsonTypeString_(_pValue)));
        }
        _outValue = 0.;
        return false;
    }

    _outValue = unsafe_yyjson_get_num(_pValue);
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    StringView&       _outValue,
    const bool        _bCheckError)
{
    JUG_ASSERT(_pValue, "Value must not be null");

    if (!unsafe_yyjson_is_str(_pValue))
    {
        if (_bCheckError)
        {
            SetError_(eSerializerError::TypeMismatch, std::format("Value is not a JSON string. It is a {}", JsonTypeString_(_pValue)));
        }
        _outValue = {};
        return false;
    }

    _outValue = { unsafe_yyjson_get_str(_pValue), unsafe_yyjson_get_len(_pValue) };
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    String&           _outValue,
    const bool        _bCheckError)
{
    StringView value;
    if (!ReadTo_(_pValue, value, _bCheckError))
    {
        _outValue = {};
        return false;
    }

    _outValue = String { value };
    return true;
}

bool JsonReader::ReadTo_(
    const yyjson_val* _pValue,
    const char*&      _outValue,
    const bool        _bCheckError)
{
    StringView value;
    if (!ReadTo_(_pValue, value, _bCheckError))
    {
        _outValue = nullptr;
        return false;
    }

    _outValue = value.data();
    return true;
}

}   // namespace jug
