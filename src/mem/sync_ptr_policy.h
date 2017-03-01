
#ifndef __MEMORY_SYNC_PTR_POLICY_H__
#define __MEMORY_SYNC_PTR_POLICY_H__

#include <cassert>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>


namespace mem
{
        
    /** 
    * \brief Default allocator used by smart pointer(s).
    * Calls "new" on template type.
    */
    template<
        class TType>
    struct default_allocator
    {
        constexpr default_allocator(
            void)
            noexcept = default;

        template<
            class TType2,
            class = typename std::enable_if<std::is_convertible<TType2 *, TType *>::value, void>::type>
            default_allocator(default_allocator<TType2> const &)
            noexcept
        {}

        template<class ...TArg>
        TType * allocate(
            TArg && ...p_args)
            const
        {
            static_assert(
                0 < sizeof(TType),
                "can't allocate an incomplete type");
            return new TType(std::forward<TArg>(p_args)...);
        }

    }; // struct default_allocator


    /**
    * \brief Default deleter used by smart pointer(s).
    * Calls "delete" on target template type pointer.
    */
    template<
        class TType>
    struct default_deleter
    {	
        constexpr default_deleter(
            void) 
            noexcept = default;

        template<
            class TType2,
            class = typename std::enable_if<std::is_convertible<TType2 *, TType *>::value, void>::type>
            default_deleter(default_deleter<TType2> const &)
            noexcept
        {}

        void free(
            TType * p_ptr) 
            const noexcept
        {	
            static_assert(
                0 < sizeof(TType),
                "can't delete an incomplete type");
            delete p_ptr;
        }

    }; // struct default_deleter

    /**
    * \brief No op deleter used by smart pointer(s).
    * Does NOT free memory, no operation performed.
    */
    template<
        class TType>
        struct noop_deleter
    {
        constexpr noop_deleter(
            void)
            noexcept = default;

        template<
            class TType2,
            class = typename std::enable_if<std::is_convertible<TType2 *, TType *>::value, void>::type>
            noop_deleter(noop_deleter<TType2> const &)
            noexcept
        {}

        void free(
            TType * p_ptr)
            const noexcept
        {}

    }; // struct noop_deleter



    /**
    * \brief Reference counter.
    */
    class ref_counter
    {

    private:
        size_t	        ref_count_;
        size_t	        ref_count_ptr_;

    public:
        ref_counter(
            void)
            noexcept
            : ref_count_(1U)
            , ref_count_ptr_(0)
        {}

        inline void increment(
            void)
            noexcept
        {
            ref_count_++;
        }

        inline size_t decrement(
            void)
            noexcept
        {
            size_t ret = ref_count_;
            ref_count_--;
            return ret;
        }

        inline void increment_ptr(
            void)
            noexcept
        {
            ref_count_ptr_++;
        }

        inline size_t decrement_ptr(
            void)
            noexcept
        {
            size_t ret = ref_count_ptr_;
            ref_count_ptr_--;
            return ret;
        }

        inline size_t count(
            void)
            const noexcept
        {
            return ref_count_;
        }

        inline size_t count_ptr(
            void)
            const noexcept
        {
            return ref_count_ptr_;
        }

    }; // class ref_counter

    /**
    * \brief Atomic reference counter.
    */
    class atomic_ref_counter
    {

    private:
        std::atomic<size_t>	        ref_count_;
        std::atomic<size_t>	        ref_count_ptr_;

    public:
        inline atomic_ref_counter(
            void)
            noexcept
            : ref_count_(1U)
            , ref_count_ptr_(0)
        {}

        inline void increment(
            void)
            noexcept
        {
            ref_count_.fetch_add(1U);
        }

        inline size_t decrement(
            void)
            noexcept
        {
            return ref_count_.fetch_sub(1U);
        }

        inline void increment_ptr(
            void)
            noexcept
        {
            ref_count_ptr_.fetch_add(1U);
        }

        inline size_t decrement_ptr(
            void)
            noexcept
        {
            return ref_count_ptr_.fetch_sub(1U);
        }

        inline size_t count(
            void)
            const noexcept
        {
            return ref_count_.load();
        }

        inline size_t count_ptr(
            void)
            const noexcept
        {
            return ref_count_ptr_.load();
        }

    }; // class atomic_ref_counter



    /**
    * \brief Pointer holder.
    */
    template <class TPtr>
    class ptr_holder
    {

    private:
        TPtr *	ptr_;

    public:
        inline ptr_holder(
            void)
            noexcept
            : ptr_(nullptr)
        {}

        inline explicit ptr_holder(
            TPtr * p_ptr)
            noexcept
            : ptr_(p_ptr)
        {}

        inline TPtr * set(
            TPtr * p_ptr)
            noexcept
        {
            auto p = ptr_;
            ptr_ = p_ptr;
            return p;
        }

        inline TPtr * get(
            void)
            const noexcept
        {
            return ptr_;
        }

    }; // class ptr_holder

    /**
    * \brief Recursive mutex protected pointer holder.
    */
    template <class TPtr>
    class ptr_holder_ts
    {

    private:
        TPtr *                          ptr_;
        mutable std::recursive_mutex    mtx_;

    public:
        inline ptr_holder_ts(
            void)
            noexcept
            : ptr_(nullptr)
            , mtx_()
        {}

        inline explicit ptr_holder_ts(
            TPtr * p_ptr)
            noexcept
            : ptr_(p_ptr)
            , mtx_()
        {}

        inline TPtr * set(
            TPtr * p_ptr)
            noexcept
        {
            std::lock_guard<std::recursive_mutex> l(mtx_);
            auto p = ptr_;
            ptr_ = p_ptr;
            return p;
        }

        inline TPtr * get(
            void)
            const noexcept
        {
            std::lock_guard<std::recursive_mutex> l(mtx_);
            return ptr_;
        }

    }; // class ptr_holder_ts

} // namespace mem

#endif // __MEMORY_SYNC_PTR_POLICY_H__
