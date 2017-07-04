
#ifndef __MEM_SYNC_PTR_H__
#define __MEM_SYNC_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif


namespace mem
{

    template<class TPtr>
    using sync_ptr_deleter      = default_deleter<TPtr>;

    template <class TPtr, template <class T> class TDeleter>
    class linked_ptr;


    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter>
    class sync_ptr final
    {

    private:
        friend class linked_ptr<TPtr, TDeleter>;
        using sync_ptr_t = sync_ptr<TPtr, TDeleter>;

    private:
        class body final
            : private TDeleter<TPtr>
        {

        private:
            TPtr *                           ptr_;
            std::atomic<std::int32_t>        ref_count_;

        public:
            body(void) noexcept
                : ptr_{ nullptr }
                , ref_count_{ 1 }
            {}
            
            template<class TPtrCompatible>
            body(TPtrCompatible * p_ptr) noexcept
                : ptr_{ p_ptr }
                , ref_count_{ 1 }
            {
                assert(p_ptr);
            }

        private:
            ~body(void) noexcept = default;

        public:
            body(body const & p_rhs) = delete;
            body(body && p_rhs) = delete;
            void operator=(body & p_arg) = delete;
            void operator=(body && p_arg) = delete;

        private:
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

            void release_this(void) noexcept
            {
                release_ptr(nullptr);
                delete this;
            }

        private:
            TPtr * set(TPtr * p_ptr) noexcept
            {
                auto p = ptr_;
                ptr_ = p_ptr;
                return p;
            }

        public:
            void ref(void)  noexcept
            {
                ref_count_.fetch_add(1, std::memory_order_release);
            }

            void unref(void) noexcept
            {
                if (ref_count_.fetch_sub(1, std::memory_order_release) == 1)
                {
                    release_this();
                }
            }

            std::int32_t ref_count(void) const noexcept
            {
                return ref_count_.load(std::memory_order_acquire);
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

        public:
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

            TPtr * get(void) const noexcept
            {
                return ptr_;
            }

        }; // class body


    private:
        body *		body_;


    public:
        sync_ptr(void) noexcept
            : body_{ new body }
        {}

        template<class TPtrCompatible>
        sync_ptr(TPtrCompatible * p_ptr) noexcept
            : body_{ new body{p_ptr} }
        {}

        sync_ptr(sync_ptr_t && p_rhs) noexcept
            : body_{ p_rhs.body_ }
        {
            p_rhs.body_ = nullptr;
        }

        sync_ptr(sync_ptr_t const & p_rhs) noexcept
            : body_{ p_rhs.body_ }
        {
            body_->ref();
        }

        ~sync_ptr(void) noexcept
        {
            if (body_)
            {
                body_->unref();
            }
        }

        sync_ptr_t & operator=(sync_ptr_t && p_rhs) noexcept
        {
            body_ = p_rhs.body_;
            p_rhs.body_ = nullptr;
            return *this;
        }

        sync_ptr_t & operator=(sync_ptr_t const & p_rhs) & noexcept
        {
            auto * tmp = p_rhs.body_;
            if (tmp != body_)
            {
                body_->unref();
                body_ = tmp;
                body_->ref();
            }
            return *this;
        }

        void swap(sync_ptr_t & p_rhs) noexcept
        {
            std::swap(body_, p_rhs.body_);
        }

    public:
        template <class TPtrCompatible>
        void reset(TPtrCompatible * p_ptr) noexcept
        {
            assert(p_ptr);
            body_->reset(p_ptr);
        }

        void reset(void) noexcept
        {
            body_->reset();
        }

        TPtr * release(void) noexcept
        {
            return body_->release();
        }

        template <class TPtrCompatible>
        TPtr * exchange(TPtrCompatible * p_ptr) noexcept
        {
            return body_->exchange(p_ptr);
        }

    public:
        TPtr * get(void) const noexcept
        {
            return body_->get();
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

        std::int32_t count(void) const noexcept
        {
            return body_->ref_count();
        }

    }; // class sync_ptr

} // namespace mem


namespace std
{
    
    template <
        class TPtr,
        template <class T> class TDeleter>
    void swap(
        mem::sync_ptr<TPtr, TDeleter> & p_lhs,
        mem::sync_ptr<TPtr, TDeleter> & p_rhs)
        noexcept
    {
        p_lhs.swap(p_rhs);
    }

} // namespace std

//=============================================================================

namespace mem
{

    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter,
        class... TArgs>
    inline 
    typename std::enable_if<!std::is_array<TPtr>::value, mem::sync_ptr<TPtr, TDeleter>>::type
    make_sync(
        TArgs&&... p_args)
    {
        TPtr * ptr = new TPtr(std::forward<TArgs>(p_args)...);

        sync_ptr<TPtr, TDeleter> res;
        res.reset(ptr);
        return res;
    }

    template<
        class TPtr,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
    make_sync(
        TArgs&&...)
        = delete;
    

    template <
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter = sync_ptr_deleter,
        class... TArgs>
    inline 
    typename std::enable_if<!std::is_array<TPtr>::value, mem::sync_ptr<TPtr, TDeleter>>::type
    allocate_sync(
        TAllocator<TPtr> & p_allocator,
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

        sync_ptr<TPtr, TDeleter> res;
        res.reset(ptr);
        return res;
    }

    template<
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type
    allocate_sync(
        TAllocator<TPtr> const & p_allocator,
        TArgs&&...)
        = delete;

} // namespace mem

//=============================================================================

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator==(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
    noexcept
{
    return (p_lhs.get() == p_rhs.get());
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator!=(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
    noexcept
{
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator<(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{
    typedef typename mem::sync_ptr<TPtr1, TDeleter1>::pointer ptr1_t;
    typedef typename mem::sync_ptr<TPtr2, TDeleter2>::pointer ptr2_t;
    typedef typename std::common_type<ptr1_t, ptr2_t>::type common_t;
    return (std::less<common_t>()(p_lhs.get(), p_rhs.get()));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator>=(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator>(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{
    return (p_rhs < p_lhs);
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator<=(
    mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{
    return (!(p_rhs < p_lhs));
}



template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator==(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t)
    noexcept
{
    return (!p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator==(
    std::nullptr_t,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
    noexcept
{
    return (!p_rhs);
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator!=(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
    noexcept
{
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator!=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
    noexcept
{
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator<(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{
    typedef typename mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs.get(), p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator<(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{
    typedef typename mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs, p_rhs.get()));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator>=(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator>=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator>(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator>(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator<=(
    mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{
    return (!(p_rhs < p_lhs));
}

template<
    class TPtr,
    template <class T> class TDeleter>
inline bool operator<=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{
    return (!(p_rhs < p_lhs));
}

#endif // __MEM_SYNC_PTR_H__
