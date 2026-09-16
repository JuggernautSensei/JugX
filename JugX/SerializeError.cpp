#include "pch.h"
#include "SerializeError.h"

namespace jug
{

StringView SerializeErrorCategory::GetName() const noexcept
{
    return "Serialize";
}

String SerializeErrorCategory::MakeMessage(int _err) const
{
    switch (static_cast<eSerializeError>(_err))
    {
        case eSerializeError::None:
            return "No error";
        case eSerializeError::ParseFailed:
            return "Parse failed";
        case eSerializeError::OutputFailed:
            return "Output failed";
        case eSerializeError::TypeMismatch:
            return "Type mismatch";
        case eSerializeError::MissingField:
            return "Missing field";
        case eSerializeError::StringifyFailed:
            return "Stringify failed";
        case eSerializeError::UnknownError:
            return "Unknown error";
        default:
            JUG_ASSERT(false, "Unrecognized error code.\n");
            return "Unrecognized error code";
    }
}

}   // namespace jug