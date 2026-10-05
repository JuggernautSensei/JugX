#pragma once

namespace jug
{

namespace callable_detail
{
    enum class eOperation
    {
        Destroy,
        Copy,
        Move,
    };

    template<typename T, size_t kSize>
    concept SmallFnT = sizeof(T) <= kSize && alignof(T) <= alignof(void*) && std::is_nothrow_move_constructible_v<T>;

    template<typename T, size_t kSize>
    concept TriviallyStorable = SmallFnT<T, kSize> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

    // storage에 객체를 직접 저장
    template<typename T, size_t kSize>
    struct SboPolicy
    {
        using StorageT = ARRAY<std::byte, kSize>;

        static T& Get(
            StorageT& _storage)
        {
            return *reinterpret_cast<T*>(_storage.data());
        }

        template<typename U>
        static void Create(
            StorageT& _storage,
            U&&       _value)
        {
            std::construct_at(reinterpret_cast<T*>(_storage.data()), std::forward<U>(_value));
        }

        static void Manage(
            const eOperation _op,
            StorageT*        _pDst,
            StorageT*        _pSrc)
        {
            switch (_op)
            {
                case eOperation::Destroy:
                {
                    std::destroy_at(&Get(*_pDst));   // 메모리는 Callable 객체가 소멸하면서 공멸
                    break;
                }

                case eOperation::Copy:
                {
                    std::construct_at(reinterpret_cast<T*>(_pDst->data()), Get(*_pSrc));
                    break;
                }

                case eOperation::Move:
                {
                    std::construct_at(reinterpret_cast<T*>(_pDst->data()), std::move(Get(*_pSrc)));
                    std::destroy_at(&Get(*_pSrc));
                    break;
                }

                default:
                    JUG_ASSERT(false, "Unknown eOperation type");
            }
        }
    };

    // 힙에 객체를 생성하고 storage에는 포인터만 저장
    template<typename T, size_t kSize>
    struct HeapPolicy
    {
        using StorageT = ARRAY<std::byte, kSize>;

        static T& Get(
            StorageT& _storage)
        {
            return *PtrRef_(_storage);
        }

        template<typename U>
        static void Create(
            StorageT& _storage,
            U&&       _value)
        {
            PtrRef_(_storage) = new T(std::forward<U>(_value));
        }

        static void Manage(
            const eOperation _op,
            StorageT*        _pDst,
            StorageT*        _pSrc)
        {
            switch (_op)
            {
                case eOperation::Destroy:
                {
                    delete PtrRef_(*_pDst);
                    break;
                }

                case eOperation::Copy:
                {
                    // 힙에 복사 생성 후 포인터만 저장
                    PtrRef_(*_pDst) = new T(*PtrRef_(*_pSrc));
                    break;
                }

                case eOperation::Move:
                {
                    // 포인터만 이동
                    PtrRef_(*_pDst) = PtrRef_(*_pSrc);
                    PtrRef_(*_pSrc) = nullptr;
                    break;
                }

                default:
                    JUG_ASSERT(false, "Unknown eOperation type");
            }
        }

    private:
        static T*& PtrRef_(
            StorageT& _storage)
        {
            return *reinterpret_cast<T**>(_storage.data());
        }
    };

    template<typename T, size_t kSize>
    using PolicyT = std::conditional_t<SmallFnT<T, kSize>, SboPolicy<T, kSize>, HeapPolicy<T, kSize>>;

}   // namespace callable_detail

// ===========================================================================
//  Callable
// ===========================================================================

template<size_t kSize, bool kbOverflow, typename>
class BaseCallable;

template<size_t kSize, bool kbOverflow, typename R, typename... Args>
class BaseCallable<kSize, kbOverflow, R(Args...)>   // NOLINT
{
    static_assert(kSize >= sizeof(void*), "kSize must be at least sizeof(void*)");

    using StorageT   = ARRAY<std::byte, kSize>;
    using CallableFn = R (*)(Args...);
    using Fn         = R (*)(StorageT& _pStorage, Args... _args);
    using VTableFn   = void (*)(callable_detail::eOperation _op, StorageT* _pDst, StorageT* _pSrc);

public:
    BaseCallable() = default;

    /* implicit */ BaseCallable(
        const nullptr_t)
        : BaseCallable()
    {
    }

    BaseCallable(
        const BaseCallable& _other)
    {
        OnCopy_(_other);
    }

    BaseCallable(
        BaseCallable&& _other) noexcept
    {
        OnMove_(_other);
    }

    BaseCallable& operator=(
        const BaseCallable& _other)
    {
        if (this != &_other)
        {
            Reset_();
            OnCopy_(_other);
        }
        return *this;
    }

    BaseCallable& operator=(
        nullptr_t)
    {
        Reset_();
        return *this;
    }

    BaseCallable& operator=(
        BaseCallable&& _other) noexcept
    {
        if (this != &_other)
        {
            Reset_();
            OnMove_(_other);
        }
        return *this;
    }

    ~BaseCallable()
    {
        Reset_();
    }

    // 함수 포인터, 글로벌/정적 함수, non-capturing 람다
    template<auto Fn>
        requires(std::is_invocable_r_v<R, decltype(Fn), Args...> && std::convertible_to<decltype(Fn), CallableFn>)
    void Connect()
    {
        Reset_();

        m_fn = &InvokeStatic_<Fn>;
    }

    // 클래스 멤버 함수 포인터. 호출자 객체의 수명을 보장하지 않음.
    template<auto Method, typename Caller>
        requires(std::is_member_function_pointer_v<decltype(Method)> && std::is_invocable_r_v<R, decltype(Method), Caller*, Args...>)
    void Connect(
        Caller* _pCaller)
    {
        JUG_ASSERT(_pCaller, "Caller instance pointer must not be null");

        Reset_();

        callable_detail::SboPolicy<Caller*, kSize>::Create(m_storage, _pCaller);   // 호출자 객체의 주소만 저장
        m_fn = &InvokeMethod_<Method, Caller>;
    }

    // 비캡쳐 + 캡쳐 람다. 함수자.
    template<typename Fn>
        requires std::is_invocable_r_v<R, std::decay_t<Fn>&, Args...> && (kbOverflow || callable_detail::SmallFnT<std::decay_t<Fn>, kSize>)
    void Connect(
        Fn&& _fn)
    {
        using DecayedFn = std::decay_t<Fn>;
        using Policy    = callable_detail::PolicyT<DecayedFn, kSize>;

        Reset_();

        Policy::Create(m_storage, std::forward<Fn>(_fn));
        m_fn = &Invoke_<Policy>;

        // 함수 포인터, non-capturing 람다 등은 관리자 없이 memcpy 복사
        if constexpr (!callable_detail::TriviallyStorable<DecayedFn, kSize>)
        {
            m_vtableFn = &Policy::Manage;
        }
    }

    void Disconnect()
    {
        Reset_();
    }

    R operator()(
        Args... args) const
    {
        JUG_ASSERT(m_fn, "BaseCallable is not connected");
        return m_fn(m_storage, std::forward<Args>(args)...);
    }

    explicit operator bool() const
    {
        return m_fn != nullptr;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const
    {
        return m_fn == nullptr;
    }

private:
    template<typename Policy>
    static R Invoke_(
        StorageT& _storage,
        Args... _args)
    {
        return Policy::Get(_storage)(std::forward<Args>(_args)...);
    }

    template<auto Fn>
    static R InvokeStatic_(
        StorageT&,
        Args... _args)
    {
        return Fn(std::forward<Args>(_args)...);
    }

    template<auto Method, typename Caller>
    static R InvokeMethod_(
        StorageT& _storage,
        Args... _args)
    {
        return (callable_detail::SboPolicy<Caller*, kSize>::Get(_storage)->*Method)(std::forward<Args>(_args)...);
    }

    void Reset_()
    {
        if (!m_fn)
        {
            return;
        }

        if (m_vtableFn)
        {
            m_vtableFn(callable_detail::eOperation::Destroy, &m_storage, nullptr);
        }

        m_fn       = nullptr;
        m_vtableFn = nullptr;
    }

    void OnCopy_(
        const BaseCallable& _other)
    {
        JUG_ASSERT(*this == nullptr, "OnCopy_ should only be called on a disconnected BaseCallable");

        if (!_other.m_fn)
        {
            return;
        }

        if (_other.m_vtableFn)
        {
            _other.m_vtableFn(callable_detail::eOperation::Copy, &m_storage, &_other.m_storage);
        }
        else
        {
            m_storage = _other.m_storage;
        }

        m_fn       = _other.m_fn;
        m_vtableFn = _other.m_vtableFn;
    }

    void OnMove_(
        BaseCallable& _other)
    {
        JUG_ASSERT(*this == nullptr, "OnMove_ should only be called on a disconnected BaseCallable");

        if (!_other.m_fn)
        {
            return;
        }

        if (_other.m_vtableFn)
        {
            _other.m_vtableFn(callable_detail::eOperation::Move, &m_storage, &_other.m_storage);
        }
        else
        {
            m_storage = _other.m_storage;
        }

        m_fn       = _other.m_fn;
        m_vtableFn = _other.m_vtableFn;

        _other.m_fn       = nullptr;
        _other.m_vtableFn = nullptr;
    }

    Fn       m_fn                                      = nullptr;
    VTableFn m_vtableFn                                = nullptr;
    alignas(alignof(void*)) mutable StorageT m_storage = {};
};

// ===========================================================================
//  Typedef
// ===========================================================================

constexpr size_t kCallableDefaultSize = 16;

template<typename FuncSignature, size_t kSize = kCallableDefaultSize, bool kbOverflow = true>
using Callable = BaseCallable<kSize, kbOverflow, FuncSignature>;

// ===========================================================================
//  FuncSignature Signature Deduction
// ===========================================================================

template<typename T>
struct FuncSignature;

template<typename R, typename... Args>
struct FuncSignature<R (*)(Args...)>
{
    using Type = R(Args...);
};

template<typename R, typename... Args>
struct FuncSignature<R (*)(Args...) noexcept>
{
    using Type = R(Args...);
};

template<typename T>
struct MethodSignature;

template<typename R, typename Caller, typename... Args>
struct MethodSignature<R (Caller::*)(Args...)>
{
    using Type = R(Args...);
};

template<typename R, typename Caller, typename... Args>
struct MethodSignature<R (Caller::*)(Args...) const>
{
    using Type = R(Args...);
};

template<typename R, typename Caller, typename... Args>
struct MethodSignature<R (Caller::*)(Args...) noexcept>
{
    using Type = R(Args...);
};

template<typename R, typename Caller, typename... Args>
struct MethodSignature<R (Caller::*)(Args...) const noexcept>
{
    using Type = R(Args...);
};

template<typename Functor>
using FunctorSignature = decltype(&std::decay_t<Functor>::operator());

// ===========================================================================
//  Callable Creation Helper
// ===========================================================================

// 함수 포인터, 글로벌/정적 함수, non-capturing 람다
template<auto Fn>
[[nodiscard]] Callable<typename FuncSignature<decltype(Fn)>::Type> Bind()
{
    using Signature = FuncSignature<decltype(Fn)>;
    using Type      = typename Signature::Type;

    Callable<Type> callable = nullptr;
    callable.template Connect<Fn>();
    return callable;
}

// 클래스 멤버 함수 포인터
template<auto Method, typename Caller>
[[nodiscard]] Callable<typename MethodSignature<decltype(Method)>::Type> Bind(
    Caller* _pCaller)
{
    using Signature = MethodSignature<decltype(Method)>;
    using Type      = typename Signature::Type;

    Callable<Type> callable = nullptr;
    callable.template Connect<Method>(_pCaller);
    return callable;
}

// 람다, functor, capturing 람다
template<typename Fn>
[[nodiscard]] Callable<typename MethodSignature<FunctorSignature<Fn>>::Type> Bind(
    Fn&& func)
{
    using Signature = MethodSignature<FunctorSignature<Fn>>;
    using Type      = typename Signature::Type;

    Callable<Type> callable = nullptr;
    callable.Connect(std::forward<Fn>(func));
    return callable;
}

}   // namespace jug
