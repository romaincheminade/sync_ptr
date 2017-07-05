
#ifndef __MEM_ATOMIC_PTR_H__
#define __MEM_ATOMIC_PTR_H__


#include <atomic>
#include <cassert>
#include <memory>
#include <type_traits>
#include <tuple>
#include <utility>


namespace mem
{

    //==========================================================================

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

    //==========================================================================

    template <
        class TPtr,
        class TDeleter = std::default_delete<TPtr>>
    class atomic_ptr final
    {

    private:
        using atomic_ptr_t  = atomic_ptr<TPtr, TDeleter>;

    private:
        std::tuple<TPtr *, TDeleter>  tuple_;


    public:
        constexpr atomic_ptr(void) noexcept
            : tuple_{}
        {}
        
        /// Creates a unique_ptr that owns nothing.
        constexpr atomic_ptr(std::nullptr_t) noexcept
            : atomic_ptr()
        {}


        atomic_ptr(atomic_ptr_t const & p_rhs) noexcept = delete;
        atomic_ptr(atomic_ptr_t && p_rhs) noexcept
            : tuple_{}
        {
            _store_release(&std::get<0>(tuple_), p_rhs.release());
            get_deleter() = std::forward<TDeleter>(p_rhs.get_deleter());
        }

        atomic_ptr_t & operator=(atomic_ptr_t const & p_rhs) & noexcept = delete;
        atomic_ptr_t & operator=(atomic_ptr_t && p_rhs) noexcept
        {
            _store_release(&std::get<0>(tuple_), p_rhs.release());
            get_deleter() = std::forward<TDeleter>(p_rhs.get_deleter());
            return *this;
        }
        

        template<
            class Tp,
            class = typename std::enable_if<std::is_convertible<Tp *, TPtr *>::value, void>::type >
        atomic_ptr(Tp * p_ptr) noexcept
            : tuple_{}
        {
            _store_release(&std::get<0>(tuple_), p_ptr);
        }

        template<
            class Tp,
            class = typename std::enable_if<std::is_convertible<Tp *, TPtr *>::value, void>::type >
        atomic_ptr(
            Tp * p_ptr,
            typename std::conditional<std::is_reference<TDeleter>::value, TDeleter, TDeleter const &>::type p_deleter)
            noexcept
            : tuple_{nullptr, p_deleter}
        {
            _store_release(&std::get<0>(tuple_), p_ptr);
        }

        template<
            class Tp,
            class = typename std::enable_if<std::is_convertible<Tp *, TPtr *>::value, void>::type >
        atomic_ptr(
            Tp * p_ptr,
            typename std::remove_reference<TDeleter>::type && p_deleter)
            noexcept
            : tuple_{nullptr, std::move(p_deleter)}
        {
            _store_release(&std::get<0>(tuple_), p_ptr);
        }


        atomic_ptr_t & operator=(std::nullptr_t) noexcept
        {
            reset();
            return *this;
        }


        template<
            class Tp,
            class Td,
            class = typename std::enable_if<std::is_convertible<typename std::unique_ptr<Tp, Td>::pointer, TPtr *>::value, void>::type >
        atomic_ptr(std::unique_ptr<Tp, Td> && p_rhs) noexcept
            : tuple_{}
        {
            _store_release(&std::get<0>(tuple_), p_rhs.release());
            get_deleter() = std::forward<TDeleter>(p_rhs.get_deleter());
        }

        template<
            class Tp,
            class Td,
            class = typename std::enable_if<std::is_convertible<typename std::unique_ptr<Tp, Td>::pointer, TPtr *>::value, void>::type >
        atomic_ptr_t & operator=(std::unique_ptr<Tp, Td> && p_rhs) noexcept
        {
            _store_release(&std::get<0>(tuple_), p_rhs.release());
            get_deleter() = std::forward<TDeleter>(p_rhs.get_deleter());
            return *this;
        }
        


        ~atomic_ptr(void) noexcept
        {
            reset();
        }
        

    private:
        TPtr * set(TPtr * p_ptr) noexcept
        {
            return _exchange_acquire_release(&std::get<0>(tuple_), p_ptr);
        }

        void reset_ptr(TPtr * p_ptr) noexcept
        {
            auto p = set(p_ptr);
            if (p)
            {
                get_deleter()(p);
            }
        }


    public:
        template<
            class Tp,
            class = typename std::enable_if<std::is_convertible<Tp *, TPtr *>::value, void>::type >
        void reset(Tp * p_ptr) noexcept
        {
            assert(p_ptr);
            assert(p_ptr != get());
            reset_ptr(p_ptr);
        }

        void reset(void) noexcept
        {
            reset_ptr(nullptr);
        }

        void reset(std::nullptr_t) noexcept
        {
            reset();
        }

        TPtr * release(void) noexcept
        {
            return set(nullptr);
        }

        template<
            class Tp,
            class = typename std::enable_if<std::is_convertible<Tp *, TPtr *>::value, void>::type >
        TPtr * exchange(Tp * p_ptr) noexcept
        {
            assert(p_ptr);
            assert(p_ptr != get());
            return set(p_ptr);
        }


    public:
        TPtr * non_atomic_get(void) const noexcept
        {
            return std::get<0>(tuple_);
        }

        TPtr * get(void) const noexcept
        {
            return _load_acquire(&std::get<0>(tuple_));
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
        /// Return a reference to the stored deleter.
        TDeleter & get_deleter(void) noexcept
        {
            return std::get<1>(tuple_);
        }

        /// Return a const reference to the stored deleter.
        const TDeleter & get_deleter(void) const noexcept
        {
            return std::get<1>(tuple_);
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
        class TDeleter = std::default_delete<TPtr>,
        class... TArgs>
    inline
    typename std::enable_if<!std::is_array<TPtr>::value, mem::atomic_ptr<TPtr, TDeleter>>::type
    make_atomic(
        TArgs&&... p_args)
    {
        TPtr * ptr = new TPtr(std::forward<TArgs>(p_args)...);

        atomic_ptr<TPtr, TDeleter> res;
        res.reset(ptr);
        return res;
    }

    template<
        class TPtr,
        class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
    make_atomic(
        TArgs&&...)
        = delete;


    template <
        class TPtr,
        class TAllocator,
        class TDeleter = std::default_delete<TPtr>,
        class... TArgs>
    inline 
    typename std::enable_if<!std::is_array<TPtr>::value, mem::atomic_ptr<TPtr, TDeleter>>::type
    allocate_atomic(
        TAllocator & p_allocator,
        TArgs&&... p_args)
    {
        TPtr * ptr = p_allocator.allocate(1);

        try {
            p_allocator.construct(ptr, std::forward<TArgs>(p_args)...);
        }
        catch (...) {
            p_allocator.deallocate(ptr, 1);
            throw;
        }

        atomic_ptr<TPtr, TDeleter> res;
        res.reset(ptr);
        return res;
    }

    template<
        class TPtr,
        class TAllocator,
        class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
    allocate_atomic(
        TAllocator const & p_allocator,
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
