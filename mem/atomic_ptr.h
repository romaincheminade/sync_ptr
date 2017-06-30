
#ifndef __MEM_ATOMIC_PTR_H__
#define __MEM_ATOMIC_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif


namespace mem
{

    template <
        class TPtr,
        template <class T> class TDeleter = default_deleter<TPtr>>
    class atomic_ptr final
        : private TDeleter<TPtr>
    {

    private:
        using atomic_ptr_t  = atomic_ptr<TPtr, TDeleter>;

    private:
        std::atomic<TPtr *>     atomic_ptr_;


    public:
        atomic_ptr(void) noexcept
            : atomic_ptr_{ nullptr }
        {}

        template<class TPtrCompatible>
        atomic_ptr(TPtrCompatible * p_ptr) noexcept
            : atomic_ptr_{ p_ptr }
        {
            assert(p_ptr);
        }

        ~atomic_ptr(void) noexcept
        {
            reset();
        }


    public:
        atomic_ptr(atomic_ptr_t && p_rhs) noexcept
            : atomic_ptr_{ p_rhs.get() }
        {}

        atomic_ptr_t & operator=(atomic_ptr_t && p_rhs) noexcept
        {
            atomic_ptr_.store(p_rhs.get(), std::memory_order_release);
            return *this;
        }

        atomic_ptr(atomic_ptr_t const & p_rhs) noexcept = delete;
        atomic_ptr_t & operator=(atomic_ptr_t const & p_rhs) & noexcept = delete;


    private:
        TPtr * set(TPtr * p_ptr) noexcept
        {
            auto p = get();
            atomic_ptr_.store(p_ptr, std::memory_order_release);
            return p;
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
        void swap(atomic_ptr_t & p_rhs) noexcept
        {
            assert(0);
            //atomic_ptr_.store(p_rhs.get(), std::memory_order_release);
            //p_rhs.atomic_ptr_.store(get(), std::memory_order_release);
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
        TPtr * get(void) const noexcept
        {
            return atomic_ptr_.load(std::memory_order_acquire);
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
        template <class T> class TDeleter = sync_ptr_deleter,
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
        template <class T> class TDeleter = sync_ptr_deleter,
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

namespace std
{
    
    template <
        class TPtr,
        template <class T> class TDeleter>
    void swap(
        mem::atomic_ptr<TPtr, TDeleter> & p_lhs,
        mem::atomic_ptr<TPtr, TDeleter> & p_rhs)
        noexcept
    {
        p_lhs.swap(p_rhs);
    }

} // namespace std

#endif // __MEM_ATOMIC_PTR_H__
