
#ifndef __MEMORY_SYNC_PTR_H__
#define __MEMORY_SYNC_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <utility>

#ifndef __MEMORY_SYNC_PTR_POLICY_H__
#include "mem/sync_ptr_policy.h"
#endif


namespace mem
{

    template<class TPtr>
    using sync_ptr_allocator    = default_allocator<TPtr>;

    template<class TPtr>
    using sync_ptr_deleter      = default_deleter<TPtr>;


    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter>
    class sync_ptr final
    {
    private:
        using sync_ptr_t = sync_ptr<TPtr, TDeleter>;

    private:
        class body final
            : private TDeleter<TPtr>
        {

        private:
            TPtr *                           ptr_;
            std::atomic<std::int32_t>        ref_count_;
            std::atomic<std::int32_t>        ref_count_ptr_;

        public:
            body(void) noexcept
                : ptr_{ nullptr }
                , ref_count_{ 1U }
                , ref_count_ptr_{ 0 }
            {}
            
            template<
                class TPtrCompatible>
            body(TPtrCompatible * p_ptr) noexcept
                : ptr_{ p_ptr }
                , ref_count_(1)
                , ref_count_ptr_(1)
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
            void release_this(void) noexcept
            {
                delete this;
            }

        private:
            TPtr * set(TPtr * p_ptr) noexcept
            {
                auto p = ptr_;
                ptr_ = p_ptr;
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
            void ref(void)  noexcept
            {
                ref_count_.fetch_add(1U, std::memory_order_release);
            }

            void unref(void) noexcept
            {
                if (ref_count_.fetch_sub(1, std::memory_order_release) == 1U)
                {
                    release_this();
                }
            }

            std::int32_t ref_count(void) const noexcept
            {
                return ref_count_.load(std::memory_order_acquire);
            }

        public:
            void ref_ptr(void) noexcept
            {
                if (get_ptr())
                {
                    ref_count_ptr_.fetch_add(1, std::memory_order_release);
                }
            }

            void unref_ptr(void) noexcept
            {
                if (get_ptr())
                {
                    if (ref_count_ptr_.fetch_add(1, std::memory_order_release) == 1U)
                    {
                        release_ptr(nullptr);
                    }
                }
            }

        public:
            template<class TPtrCompatible>
            void reset_ptr(TPtrCompatible * p_ptr) noexcept
            {
                assert(p_ptr);
                assert(p_ptr != get_ptr());
                release_ptr(p_ptr);
            }

            void reset_ptr(void) noexcept
            {
                release_ptr(nullptr);
            }

            std::int32_t ref_count_ptr(void) const noexcept
            {
                return ref_count_ptr_.load(std::memory_order_acquire);
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
                assert(p_ptr != get_ptr());
                return set(p_ptr);
            }

            TPtr * get_ptr(void) const noexcept
            {
                return ptr_;
            }

        }; // class body


    private:
        body *		body_;


    public:
        sync_ptr(void) noexcept
            : body_(new body())
        {}

        template<class TPtrCompatible>
        sync_ptr(TPtrCompatible * p_ptr) noexcept
            : body_(new body(p_ptr))
        {}

        sync_ptr(sync_ptr && p_rhs) noexcept
            : body_(p_rhs.body_)
        {
            p_rhs.body_ = nullptr;
        }

        sync_ptr(sync_ptr const & p_rhs) noexcept
            : body_(p_rhs.body_)
        {
            body_->ref();
            body_->ref_ptr();
        }

        ~sync_ptr(void) noexcept
        {
            if (body_)
            {
                body_->unref_ptr();
                body_->unref();
            }
        }

        sync_ptr_t & operator=(sync_ptr_t && p_rhs) noexcept
        {
            auto * tmp = p_rhs.body_;
            if (tmp != body_)
            {
                body_ = tmp;
                p_rhs.body_ = nullptr;
            }
            return *this;
        }

        sync_ptr_t & operator=(sync_ptr_t const & p_rhs) & noexcept
        {
            auto * tmp = p_rhs.body_;
            if (tmp != body_)
            {
                body_->unref_ptr();
                body_->unref();

                body_ = tmp;
                body_->ref();
                body_->ref_ptr();
            }
            return *this;
        }

        void swap(sync_ptr & p_rhs) noexcept
        {
            std::swap(body_, p_rhs.body_);
        }

    public:
        std::int32_t count(void) const noexcept
        {
            return body_->ref_count_ptr();
        }

    public:
        template <class TPtrCompatible>
        void reset(TPtrCompatible * p_ptr) noexcept
        {
            assert(p_ptr);
            body_->reset_ptr(p_ptr);
        }

        void reset(void) noexcept
        {
            body_->reset_ptr();
        }

    public:
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
            return body_->get_ptr();
        }

        TPtr & operator*(void) const noexcept
        {
            return *get();
        }

        TPtr * operator->(void) const noexcept
        {
            return get();
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


    ///////////////////////////////////////////////////////////////////////////////////////////
    //		MAKE
    ///////////////////////////////////////////////////////////////////////////////////////////

namespace mem
{

    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value,
        mem::sync_ptr<TPtr, TDeleter>>::type
        make_sync(
            TArgs&&... p_args)
    {
        return (sync_ptr<TPtr, TDeleter>(
                new TPtr(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync(
            TArgs&&...)
        = delete;


    ///////////////////////////////////////////////////////////////////////////////////////////
    //		MAKE WITH ALLOCATOR
    ///////////////////////////////////////////////////////////////////////////////////////////

    template <
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter = sync_ptr_deleter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value,
        mem::sync_ptr<TPtr, TDeleter>>::type
        make_sync_with_allocator(
            TAllocator<TPtr> const & p_allocator,
            TArgs&&... p_args)
    {
        return (sync_ptr<TPtr, TDeleter>(
                p_allocator.allocate(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync_with_allocator(
        TAllocator<TPtr> const & p_allocator,
        TArgs&&...)
        = delete;

} // namespace mem

///////////////////////////////////////////////////////////////////////////////////////////////////
//      RELATIONAL OPERATORS
///////////////////////////////////////////////////////////////////////////////////////////////////

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

#endif // __MEMORY_SYNC_PTR_H__
