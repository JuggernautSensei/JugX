#pragma once
#include <source_location>

#include "Typedef.h"

namespace jug
{

void Assertion(bool _bCondition, StringView _msg, std::source_location _loc);

}
