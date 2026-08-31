#include "Assertion.h"

#include <format>
#include <string>

namespace jug
{

void Assertion(
    const bool                 _bCondition,
    const StringView     _msg,
    const std::source_location _loc)
{
    if (!_bCondition)
    {

    }
}

}   // namespace jug