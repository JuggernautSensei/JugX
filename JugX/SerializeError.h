#pragma once
#include "Error.h"
#include "Math.h"

namespace jug
{

enum class eSerializeError
{
    None = 0,

    ParseFailed,       // 입력이 올바른 포맷이 아님
    OutputFailed,      // 출력이 올바른 포맷이 아님
    TypeMismatch,      // 요청한 타입과 실제 값의 타입이 다름
    MissingField,      // object에 해당 key가 없음
    StringifyFailed,   // 직렬화 출력 실패
    UnknownError,
};

class SerializeErrorCategory : public IErrorCategory
{
public:
    using ErrorT = eSerializeError;

    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeErrorMessage(int _err) const override;
};

JUG_DEFINE_ERROR_ENUM(eSerializeError, SerializeErrorCategory);

}   // namespace jug
