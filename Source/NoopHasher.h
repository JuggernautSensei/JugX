#pragma once
#include <type_traits>

namespace jug
{

// 해쉬 테이블을 설계할 때 이미 잘 해싱된 값을 키로 사용할 경우
// NoopHasher를 사용하여 해시 계산을 건너뛸 수 있음.

template<typename T>
    requires std::is_constructible_v<T, size_t>
class NoopHasher
{
public:
    [[nodiscard]] constexpr size_t operator()(
        const T& _value) const noexcept
    {
        return static_cast<size_t>(_value);
    }
};

}   // namespace jug