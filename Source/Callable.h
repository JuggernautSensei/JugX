#pragma once
#include <array>
#include <cstddef>
#include <memory>

#include "Config.h"

#define DT callable_detail

namespace jug
{

namespace DT
{
    constexpr size_t kStorageAlign = alignof(void*);

    template<size_t kStorageSize>
    using Storage = std::array<std::byte, kStorageSize> alignas(kStorageAlign);

    template<typename Fn, size_t kStorageSize, typename DecayedFn = std::decay_t<Fn>>
    concept SmallBufferOptimizableFnT = sizeof(DecayedFn) <= kStorageSize && alignof(DecayedFn) <= kStorageAlign;

    enum class eVTableOp
    {
        Destroy,
        Copy,
        Move,
    };

    using VTableFn = void (*)(eVTableOp _op, void* _pDst, void* _pSrc);

    template<typename Fn, size_t kStorageSize>
    void VTableFnImpl(
        const eVTableOp _op,
        void*           _pDst,
        void*           _pSrc)
    {
        using Storage   = Storage<kStorageSize>;
        using DecayedFn = std::decay_t<Fn>;

        switch (_op)
        {
            case eVTableOp::Destroy:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                if constexpr (SmallBufferOptimizableFnT<DecayedFn, kStorageSize>)
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

            case eVTableOp::Copy:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                Storage* pSrc = static_cast<Storage*>(_pSrc);
                if constexpr (SmallBufferOptimizableFnT<DecayedFn, kStorageSize>)
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

            case eVTableOp::Move:
            {
                Storage* pDst = static_cast<Storage*>(_pDst);
                Storage* pSrc = static_cast<Storage*>(_pSrc);
                if constexpr (SmallBufferOptimizableFnT<DecayedFn, kStorageSize>)
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
                JUG_ASSERT(false, "Unknown eVTableOp type");
        }
    }

}   // namespace DT

// ===========================================================================
//  Callable
// ===========================================================================

template<size_t kStorageSize, bool kbAllowHeapAlloc, typename>
class BaseCallable;

template<size_t kStorageSize, bool kbAllowHeapAlloc, typename TReturn, typename... TArgs>
class BaseCallable<kStorageSize, kbAllowHeapAlloc, TReturn(TArgs...)>   // NOLINT
{
    static_assert(kStorageSize >= sizeof(void*), "kStorageSize must be at least sizeof(void*)");

    using Storage    = DT::Storage<kStorageSize>;
    using CallableFn = TReturn (*)(TArgs...);
    using DelegateFn = TReturn (*)(Storage& _pStorage, TArgs... _args);

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
        requires(std::is_invocable_r_v<TReturn, decltype(Fn), TArgs...> && std::convertible_to<decltype(Fn), CallableFn>)
    void Connect()
    {
        Reset_();

        m_vtableFn   = nullptr;
        m_delegateFn = [](Storage&, TArgs... _args) -> TReturn {
            return Fn(std::forward<TArgs>(_args)...);   // 단순 함수 호출
        };
    }

    // 클래스 멤버 함수 포인터. 호출자 객체의 수명을 보장하지 않음.
    template<auto Method, typename TCaller>
        requires(std::is_member_function_pointer_v<decltype(Method)> && std::is_invocable_r_v<TReturn, decltype(Method), TCaller*, TArgs...>)
    void Connect(
        TCaller* _pCaller)
    {
        JUG_ASSERT(_pCaller, "Caller instance pointer must not be null");

        Reset_();

        *reinterpret_cast<TCaller**>(m_storage.data()) = _pCaller;   // 호출자 객체의 주소만 저장
        m_vtableFn                                     = nullptr;
        m_delegateFn                                   = [](Storage& _pStorage, TArgs... _args) -> TReturn {
            return (*reinterpret_cast<TCaller**>(_pStorage.data())->*Method)(std::forward<TArgs>(_args)...);
        };
    }

    // 비캡쳐 + 캡쳐 람다. 함수자.
    template<typename Fn>
        requires std::is_invocable_r_v<TReturn, Fn, TArgs...> && (kbAllowHeapAlloc || DT::SmallBufferOptimizableFnT<Fn, kStorageSize>)
    void Connect(
        Fn&& func)
    {
        Reset_();

        if constexpr (std::constructible_from<Fn, CallableFn>)   // 함수 포인터 최적화
        {
            *reinterpret_cast<CallableFn*>(m_storage.data()) = func;
            m_vtableFn                                       = nullptr;
            m_delegateFn                                     = [](Storage& _pStorage, TArgs... _args) -> TReturn {
                return (*reinterpret_cast<CallableFn*>(_pStorage.data()))(std::forward<TArgs>(_args)...);
            };
        }
        else
        {
            using DecayedFn = std::decay_t<Fn>;

            if constexpr (DT::SmallBufferOptimizableFnT<Fn, kStorageSize>)
            {
                // SBO. storage에 직접 객체를 생성
                std::construct_at(reinterpret_cast<DecayedFn*>(m_storage.data()), std::forward<Fn>(func));
                m_delegateFn = [](Storage& _pStorage, TArgs... _args) -> TReturn {
                    return (*reinterpret_cast<DecayedFn*>(_pStorage.data()))(std::forward<TArgs>(_args)...);
                };
            }
            else
            {
                // 힙 할당. storage에 포인터를 저장
                DecayedFn* pHeap                                 = new DecayedFn(std::forward<Fn>(func));
                *reinterpret_cast<DecayedFn**>(m_storage.data()) = pHeap;
                m_delegateFn                                     = [](Storage& _pStorage, TArgs... _args) {
                    return (**reinterpret_cast<DecayedFn**>(_pStorage.data()))(std::forward<TArgs>(_args)...);
                };
            }

            // vtable 설정
            m_vtableFn = DT::VTableFnImpl<Fn, kStorageSize>;
        }
    }

    void Disconnect()
    {
        Reset_();
    }

    [[nodiscard]] TReturn operator()(
        TArgs... args) const
    {
        JUG_ASSERT(m_delegateFn != nullptr, "BaseCallable is not connected");
        return m_delegateFn(const_cast<Storage&>(m_storage), std::forward<TArgs>(args)...);
    }

    explicit operator bool() const
    {
        return m_delegateFn != nullptr;
    }

    [[nodiscard]] bool operator==(
        const nullptr_t) const
    {
        return m_delegateFn == nullptr;
    }

    [[nodiscard]] bool operator!=(
        const nullptr_t) const
    {
        return m_delegateFn != nullptr;
    }

private:
    void Reset_()
    {
        if (!m_delegateFn)
        {
            return;
        }

        if (m_vtableFn)
        {
            m_vtableFn(DT::eVTableOp::Destroy, &m_storage, nullptr);
        }

        m_storage    = {};
        m_delegateFn = nullptr;
        m_vtableFn   = nullptr;
    }

    void OnCopy_(
        const BaseCallable& _other)
    {
        JUG_ASSERT(*this == nullptr, "OnCopy_ should only be called on a disconnected BaseCallable");

        if (!_other.m_delegateFn)
        {
            return;
        }

        if (_other.m_vtableFn)
        {
            _other.m_vtableFn(DT::eVTableOp::Copy, &m_storage, const_cast<Storage*>(&_other.m_storage));
        }
        else
        {
            m_storage = _other.m_storage;
        }

        m_delegateFn = _other.m_delegateFn;
        m_vtableFn   = _other.m_vtableFn;
    }

    void OnMove_(
        BaseCallable& _other)
    {
        JUG_ASSERT(*this == nullptr, "OnMove_ should only be called on a disconnected BaseCallable");

        if (_other.m_delegateFn == nullptr)
        {
            return;
        }

        if (_other.m_vtableFn)
        {
            _other.m_vtableFn(DT::eVTableOp::Move, &m_storage, &_other.m_storage);
        }
        else
        {
            m_storage = _other.m_storage;
        }

        m_delegateFn        = _other.m_delegateFn;
        m_vtableFn          = _other.m_vtableFn;
        _other.m_delegateFn = nullptr;
        _other.m_vtableFn   = nullptr;
    }

    DelegateFn   m_delegateFn = nullptr;
    DT::VTableFn m_vtableFn   = nullptr;
    Storage      m_storage;
};

// ===========================================================================
//  Typedef
// ===========================================================================

constexpr size_t kCallableDefaultStorageSize = 16;

template<typename FuncSignature, size_t kFunctorStorageSize = kCallableDefaultStorageSize, bool kbAllowHeapAlloc = true>
using Callable = BaseCallable<kFunctorStorageSize, kbAllowHeapAlloc, FuncSignature>;

// 고정 형태를 쓰고 싶다면.
// template<typename FuncSignature>
// using CallableF16 = BaseCallable<16 /* SBO 16바이트 제한 */ , false /* Overflow시 힙할당 불허 */, FuncSignature>;

// ===========================================================================
//  Function Signature Deduction
// ===========================================================================

template<typename T>
struct FuncSignature;

template<typename TReturn, typename... TArgs>
struct FuncSignature<TReturn (*)(TArgs...)>
{
    using Type = TReturn(TArgs...);
};

template<typename T>
struct MethodSignature;

template<typename TReturn, typename TClass, typename... TArgs>
struct MethodSignature<TReturn (TClass::*)(TArgs...)>
{
    using Type = TReturn(TArgs...);
};

template<typename TReturn, typename TClass, typename... TArgs>
struct MethodSignature<TReturn (TClass::*)(TArgs...) const>
{
    using Type = TReturn(TArgs...);
};

template<typename Functor>
using FunctorMethodT = decltype(&std::decay_t<Functor>::operator());

// ===========================================================================
//  Callable Creation Helper
// ===========================================================================

// 함수 포인터, 글로벌/정적 함수, non-capturing 람다
template<auto Fn, typename FuncSignature = FuncSignature<decltype(Fn)>>
[[nodiscard]] auto Bind()
{
    Callable<typename FuncSignature::Type> callable = {};
    callable.template Connect<Fn>();
    return callable;
}

// 클래스 멤버 함수 포인터
template<auto Method, typename TCaller, typename FuncSignature = MethodSignature<decltype(Method)>>
[[nodiscard]] auto Bind(
    TCaller* _pCaller)
{
    Callable<typename FuncSignature::Type> callable = {};
    callable.template Connect<Method>(_pCaller);
    return callable;
}

// 람다, functor, capturing 람다
template<typename Fn, typename FuncSignature = MethodSignature<FunctorMethodT<Fn>>>
[[nodiscard]] auto Bind(
    Fn&& func)
{
    Callable<typename FuncSignature::Type> callable = {};
    callable.Connect(std::forward<Fn>(func));
    return callable;
}

}   // namespace jug

#undef DT