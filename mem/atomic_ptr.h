
#ifndef __MEM_ATOMIC_PTR_H__
#define __MEM_ATOMIC_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif


namespace mem
{
    template <typename T>
    inline void _store_release(volatile T * p_dest, T p_value)
    {
        std::atomic_thread_fence(std::memory_order_release);
        *p_dest = p_value;
    }

    template<typename T>
    inline T _load_acquire(volatile T * p_src)
    {
        T res = *p_src;
        std::atomic_thread_fence(std::memory_order_acquire);
        return res;
    }

    template <typename T>
    inline T _exchange_acquire_release(volatile T * p_dest, T p_value)
    {
#if defined(__linux__)
        return __atomic_exchange_n(p_dest, p_value, __ATOMIC_ACQ_REL);

#elif defined(_WIN32)
    #if defined(__x86_64__) || defined (_M_X64)
        return (T)_InterlockedExchange64((volatile long long *)(p_dest), (long long)(p_value));

    #else
        return (T)_InterlockedExchange((volatile long *)p_dest, (long)p_value));
    #endif

#else
        T res = _load_acquire(p_dest);
        _store_release(p_dest, p_value);
        return res;

#endif
    }


    template<class TPtr>
    using atomic_ptr_deleter = default_deleter<TPtr>;

    template <
        class TPtr,
        template <class T> class TDeleter = atomic_ptr_deleter>
    class atomic_ptr final
        : private TDeleter<TPtr>
    {

    private:
        using atomic_ptr_t  = atomic_ptr<TPtr, TDeleter>;

    private:
        TPtr *      ptr_;


    public:
        atomic_ptr(void) noexcept
            : ptr_{ nullptr }
        {}

        template<class TPtrCompatible>
        atomic_ptr(TPtrCompatible * p_ptr) noexcept
            : ptr_{ nullptr }
        {
            assert(p_ptr);
            _store_release(&ptr_, p_ptr);
        }

        ~atomic_ptr(void) noexcept
        {
            reset();
        }


    public:
        atomic_ptr(atomic_ptr_t && p_rhs) noexcept
            : ptr_{ nullptr }
        {
            _store_release(&ptr_, p_rhs.get());
        }

        atomic_ptr_t & operator=(atomic_ptr_t && p_rhs) noexcept
        {
            _store_release(&ptr_, p_rhs.get());
            return *this;
        }

        atomic_ptr(atomic_ptr_t const & p_rhs) noexcept = delete;
        atomic_ptr_t & operator=(atomic_ptr_t const & p_rhs) & noexcept = delete;


    private:
        TPtr * set(TPtr * p_ptr) noexcept
        {
            return _exchange_acquire_release(&ptr_, p_ptr);
        }

        void release_ptr(TPtr * p_ptr) noexcept
        {
            static_assert(
                noexcept(this->deallocate(p_ptr)),
                "Deleter policy must offer no-throw guarantee.");

            auto p = set(p_ptr);
            if (p)
            {
                this->deallocate(p);
            }
        }


    public:
        template<class TPtrCompatible>
        void reset(TPtrCompatible * p_ptr) noexcept
        {
            assert(p_ptr);
            assert(p_ptr != get());
            release_ptr(p_ptr);
        }

        void reset(void) noexcept
        {
            release_ptr(nullptr);
        }

        TPtr * release(void) noexcept
        {
            return set(nullptr);
        }

        template<class TPtrCompatible>
        TPtr * exchange(TPtrCompatible * p_ptr) noexcept
        {
            assert(p_ptr);
            assert(p_ptr != get());
            return set(p_ptr);
        }


    public:
        TPtr * non_atomic_get(void) const noexcept
        {
            return ptr_;
        }

        TPtr * get(void) const noexcept
        {
            return _load_acquire(&ptr_);
        }

        TPtr * operator->(void) const noexcept
        {
            return get();
        }

        TPtr & operator*(void) const noexcept
        {
            return *get();
        }


    public:
        bool valid(void) const noexcept
        {
            return (get() != nullptr);
        }

        operator bool(void) const noexcept
        {
            return valid();
        }

    }; // class atomic_ptr

} // namespace mem

//=============================================================================

namespace mem
{

    template <
        class TPtr,
        template <class T> class TDeleter = mem::atomic_ptr_deleter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value,
        mem::atomic_ptr<TPtr, TDeleter>>::type
        make_atomic(
            TArgs&&... p_args)
    {
        return (atomic_ptr<TPtr, TDeleter>(
                new TPtr(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
        make_atomic(
            TArgs&&...)
            = delete;


    template <
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter = mem::atomic_ptr_deleter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value,
        mem::atomic_ptr<TPtr, TDeleter>>::type
        allocate_atomic(
            TAllocator<TPtr> const & p_allocator,
            TArgs&&... p_args)
    {
        return (atomic_ptr<TPtr, TDeleter>(
                p_allocator.allocate(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
        allocate_atomic(
            TAllocator<TPtr> const & p_allocator,
            TArgs&&...)
            = delete;

} // namespace mem

//=============================================================================

//namespace std
//{
//
//    template <
//        class TPtr,
//        template <class T> class TDeleter>
//    void swap(
//        mem::atomic_ptr<TPtr, TDeleter> & p_lhs,
//        mem::atomic_ptr<TPtr, TDeleter> & p_rhs)
//        noexcept
//    {
//        p_lhs.swap(p_rhs);
//    }
//
//} // namespace std

#endif // __MEM_ATOMIC_PTR_H__
