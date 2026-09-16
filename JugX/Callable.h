#pragma once

namespace jug
{

namespace callable_detail
{
    template<size_t kSize>
    using Storage = ARRAY<std::byte, kSize> alignas(alignof(void*));

    template<typename Fn, size_t kSize, typename DecayedFn = std::decay_t<Fn>>
    concept SmallFnT = sizeof(DecayedFn) <= kSize && alignof(DecayedFn) <= alignof(void*);

    enum class eOperation
    {
        Destroy,
        Copy,
        Move,
    };

    using VTableFn = void (*)(eOperation _op, void* _pDst, void* _pSrc);

    template<typename Fn, size_t kSize>
    void VTableImpl(
        const eOperation _op,
        void*            _pDst,
        void*            _pSrc)
    {
        using Storage   = Storage<kSize>;
        using DecayedFn = std::decay_t<Fn>;

        switch (_op)
        {
            case eOperation::Destroy:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                if constexpr (SmallFnT<DecayedFn, kSize>)
                {
                    DecayedFn* pObj = reinterpret_cast<DecayedFn*>(pDst->data());
                    std::destroy_at(pObj);   // 객체 소멸자 호출. 메모리는 Callable 객체가 소멸하면서 공멸
                }
                else
                {
                    DecayedFn* pObj = *reinterpret_cast<DecayedFn**>(pDst->data());
                    delete pObj;   // 객체가 힙에 저장되어 있기 때문에 delete로 소멸
                }
                break;
            }

            case eOperation::Copy:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                Storage* pSrc = static_cast<Storage*>(_pSrc);
                if constexpr (SmallFnT<DecayedFn, kSize>)
                {
                    // pSrc에 객체가 직접 저장되어 있기에, pDst측에 복사 생성.
                    DecayedFn* pSrcObj = reinterpret_cast<DecayedFn*>(pSrc->data());
                    std::construct_at(reinterpret_cast<DecayedFn*>(pDst->data()), *pSrcObj);
                }
                else
                {
                    // pSrc에 힙 포인터가 저장되어 있기에 힙에 복사 생성 후 포인터만 저장
                    DecayedFn* pSrcObj                           = *reinterpret_cast<DecayedFn**>(pSrc->data());
                    DecayedFn* pDstObj                           = new DecayedFn(*pSrcObj);
                    *reinterpret_cast<DecayedFn**>(pDst->data()) = pDstObj;
                }
                break;
            }

            case eOperation::Move:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                Storage* pSrc = static_cast<Storage*>(_pSrc);
                if constexpr (SmallFnT<DecayedFn, kSize>)
                {
                    // pSrc에 객체가 직접 저장되어 있기에, pDst측에 이동 생성.
                    DecayedFn* pSrcObj = reinterpret_cast<DecayedFn*>(pSrc->data());
                    std::construct_at(reinterpret_cast<DecayedFn*>(pDst->data()), std::move(*pSrcObj));
                    std::destroy_at(pSrcObj);
                }
                else
                {
                    // storage에 heap 포인터가 저장되어있음. 포인터만 이동
                    DecayedFn* pSrcObj                           = *reinterpret_cast<DecayedFn**>(pSrc->data());
                    *reinterpret_cast<DecayedFn**>(pDst->data()) = pSrcObj;
                    *reinterpret_cast<DecayedFn**>(pSrc->data()) = nullptr;
                }
                break;
            }

            default:
                JUG_ASSERT(false, "Unknown eOperation type");
        }
    }
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

    using StorageT   = callable_detail::Storage<kSize>;
    using CallableFn = R (*)(Args...);
    using Fn         = R (*)(StorageT& _pStorage, Args... _args);

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

        m_vtableFn = nullptr;
        m_fn       = [](StorageT&, Args... _args) -> R {
            return Fn(std::forward<Args>(_args)...);   // 단순 함수 호출
        };
    }

    // 클래스 멤버 함수 포인터. 호출자 객체의 수명을 보장하지 않음.
    template<auto Method, typename Caller>
        requires(std::is_member_function_pointer_v<decltype(Method)> && std::is_invocable_r_v<R, decltype(Method), Caller*, Args...>)
    void Connect(
        Caller* _pCaller)
    {
        JUG_ASSERT(_pCaller, "Caller instance pointer must not be null");

        Reset_();

        *reinterpret_cast<Caller**>(m_storage.data()) = _pCaller;   // 호출자 객체의 주소만 저장
        m_vtableFn                                    = nullptr;
        m_fn                                          = [](StorageT& _pStorage, Args... _args) -> R {
            return (*reinterpret_cast<Caller**>(_pStorage.data())->*Method)(std::forward<Args>(_args)...);
        };
    }

    // 비캡쳐 + 캡쳐 람다. 함수자.
    template<typename Fn>
        requires std::is_invocable_r_v<R, Fn, Args...> && (kbOverflow || callable_detail::SmallFnT<Fn, kSize>)
    void Connect(
        Fn&& _fn)
    {
        Reset_();

        if constexpr (std::constructible_from<Fn, CallableFn>)   // 함수 포인터 최적화
        {
            *reinterpret_cast<CallableFn*>(m_storage.data()) = _fn;
            m_vtableFn                                       = nullptr;
            m_fn                                             = [](StorageT& _pStorage, Args... _args) -> R {
                return (*reinterpret_cast<CallableFn*>(_pStorage.data()))(std::forward<Args>(_args)...);
            };
        }
        else
        {
            using DecayedFn = std::decay_t<Fn>;

            if constexpr (callable_detail::SmallFnT<Fn, kSize>)
            {
                // SBO. storage에 직접 객체를 생성
                std::construct_at(reinterpret_cast<DecayedFn*>(m_storage.data()), std::forward<Fn>(_fn));
                m_fn = [](StorageT& _pStorage, Args... _args) -> R {
                    return (*reinterpret_cast<DecayedFn*>(_pStorage.data()))(std::forward<Args>(_args)...);
                };
            }
            else
            {
                // 힙 할당. storage에 포인터를 저장
                DecayedFn* pHeap                                 = new DecayedFn(std::forward<Fn>(_fn));
                *reinterpret_cast<DecayedFn**>(m_storage.data()) = pHeap;
                m_fn                                             = [](StorageT& _pStorage, Args... _args) {
                    return (**reinterpret_cast<DecayedFn**>(_pStorage.data()))(std::forward<Args>(_args)...);
                };
            }

            // vtable 설정
            m_vtableFn = callable_detail::VTableImpl<Fn, kSize>;
        }
    }

    void Disconnect()
    {
        Reset_();
    }

    [[nodiscard]] R operator()(
        Args... args) const
    {
        JUG_ASSERT(m_fn, "BaseCallable is not connected");
        return m_fn(const_cast<StorageT&>(m_storage), std::forward<Args>(args)...);
    }

    explicit operator bool() const
    {
        return m_fn;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const
    {
        return m_fn == nullptr;
    }

    [[nodiscard]] bool operator!=(
        const nullptr_t) const
    {
        return m_fn;
    }

private:
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

        m_storage  = {};
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
            _other.m_vtableFn(callable_detail::eOperation::Copy, &m_storage, const_cast<StorageT*>(&_other.m_storage));
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

        if (_other.m_fn == nullptr)
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

        m_fn              = _other.m_fn;
        m_vtableFn        = _other.m_vtableFn;
        _other.m_fn       = nullptr;
        _other.m_vtableFn = nullptr;
    }

    Fn                        m_fn       = nullptr;
    callable_detail::VTableFn m_vtableFn = nullptr;
    StorageT                  m_storage;
};

// ===========================================================================
//  Typedef
// ===========================================================================

constexpr size_t kCallableDefaultSize = 16;

template<typename FuncSignature, size_t kSize = kCallableDefaultSize, bool kbOverflow = true>
using Callable = BaseCallable<kSize, kbOverflow, FuncSignature>;

// ===========================================================================
//  Fn Signature Deduction
// ===========================================================================

template<typename T>
struct FuncSignature;

template<typename R, typename... Args>
struct FuncSignature<R (*)(Args...)>
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

template<typename Functor>
using FunctorSignature = decltype(&std::decay_t<Functor>::operator());

// ===========================================================================
//  Callable Creation Helper
// ===========================================================================

// 함수 포인터, 글로벌/정적 함수, non-capturing 람다
template<auto Fn, typename Signature = FuncSignature<decltype(Fn)>>
[[nodiscard]] auto Bind()
{
    Callable<typename Signature::Type> callable = {};
    callable.template Connect<Fn>();
    return callable;
}

// 클래스 멤버 함수 포인터
template<auto Method, typename Caller, typename Signature = MethodSignature<decltype(Method)>>
[[nodiscard]] auto Bind(
    Caller* _pCaller)
{
    Callable<typename Signature::Type> callable = {};
    callable.template Connect<Method>(_pCaller);
    return callable;
}

// 람다, functor, capturing 람다
template<typename Fn, typename Signature = MethodSignature<FunctorSignature<Fn>>>
[[nodiscard]] auto Bind(
    Fn&& func)
{
    Callable<typename Signature::Type> callable = {};
    callable.Connect(std::forward<Fn>(func));
    return callable;
}

}   // namespace jug
