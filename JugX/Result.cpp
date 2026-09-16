#include "pch.h"
#include "Result.h"

#include "Error.h"

namespace jug
{

namespace result_detail
{

    bool IsError(
        const Error _error)
    {
        return _error.IsError();
    }

    Error GetError(
        const Error _error)
    {
        return _error;
    }

}   // namespace result_detail

}   // namespace jug