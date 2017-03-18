
#ifndef __CC_SYNC_PTR_H__
#define __CC_SYNC_PTR_H__

#include <cassert>
#include <atomic>
#include <functional>
#include <memory>

#ifndef __MEMORY_SYNC_PTR_POLICY_H__
#include "mem/sync_ptr_policy.h"
#endif


namespace cc
{
    template<class TPtr>
    using sync_ptr_allocator    = mem::default_allocator<TPtr>;

    template<class TPtr>
    using sync_ptr_deleter      = mem::default_deleter<TPtr>;

    /**
    * \class cc::sync_ptr
    *
    * \brief Reference counted synchronized pointer.
    * Used to avoid cross module cycle.
    * When the original sync_ptr or one of its copy underlying raw pointer changes,
    * all sync_ptr and copies point to the updated raw pointer.
    */
    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter >
    class sync_ptr final
    {

    public:
        typedef typename TPtr		    pointer_type;
        typedef typename TDeleter<TPtr>	deleter_type;


    private:
        /**
        * \class cc::sync_ptr::body
        *
        * \brief Reference counted template type pointer.
        * Deletes pointer when reference count drops to zero.
        * Reference count and pointer are atomic.
        */
        template<
            class TPtr,
            template <class T> class TDeleter>
        class body final
            : private TDeleter<TPtr>
        {

            //////////////////////////////////////
            //              MEMBERS             //
            //////////////////////////////////////

        private:
            std::atomic<size_t>	        ref_count_;
            std::atomic<size_t>	        ref_count_ptr_;
            std::atomic<TPtr *>		    ptr_;


            //////////////////////////////////////
            //              METHODS             //
            //////////////////////////////////////

        public:
            body(body const & p_other) = delete;
            body(body && p_other) = delete;
            void operator=(body & p_arg) = delete;
            void operator=(body && p_arg) = delete;

        public:
            body(
                void)
                noexcept
                // Members.
                : ref_count_(0)
                , ref_count_ptr_(0)
                , ptr_(nullptr)
            {}

            template<
                class TPtrCompatible>
            body(
                TPtrCompatible * p_ptr)
                noexcept
                // Members.
                : ref_count_(1U)
                , ref_count_ptr_(1U)
                , ptr_(p_ptr)
            {
                assert(p_ptr);
            }

        private:
            ~body(
                void)
                noexcept
            {}


        private:
            /**
            * \brief Delete this.
            * Called when this reference count drops to zero.
            */
            inline void release_this(
                void)
                noexcept
            {
                delete this;
            }
            /**
            * \brief Delete contained pointer and store target one using CAS.
            */
            inline bool release_ptr_cas(
                TPtr * p_ptr)
                noexcept
            {
                auto ptr = ptr_.load();
                if (ptr_.compare_exchange_strong(
                    ptr,
                    p_ptr))
                {
                    if (ptr)
                    {
                        free(ptr);
                    }
                    return true;
                }
                return false;
            }


        public:
            /**
            * \brief Increments this reference count.
            */
            inline void ref(
                void)
                noexcept
            {
                ref_count_.fetch_add(1U);
            }
            /**
            * \brief Decrements this reference count,
            * release this if reference count drops to zero.
            */
            inline void unref(
                void)
                noexcept
            {
                if (ref_count_.fetch_sub(1U) == 1U)
                {
                    release_this();
                }
            }

            /**
            * \brief Increments pointer reference count.
            */
            inline void ref_ptr(
                void)
                noexcept
            {
                if (ptr_.load())
                {
                    ref_count_ptr_.fetch_add(1U);
                }
            }
            /**
            * \brief Decrements pointer reference count,
            * release pointer if reference count drops to zero.
            */
            inline void unref_ptr(
                void)
                noexcept
            {
                if (ptr_.load())
                {
                    if (ref_count_ptr_.fetch_sub(1U) == 1U)
                    {
                        release_ptr_cas(nullptr);
                    }
                }
            }


            ///////////////////////////////////////////////////////////////////////////////////////
            //		GET / SET
            ///////////////////////////////////////////////////////////////////////////////////////

        public:
            inline size_t get_ref_count(
                void)
                const noexcept
            {
                return ref_count_.load();
            }
            inline size_t get_ref_count_ptr(
                void)
                const noexcept
            {
                return ref_count_ptr_.load();
            }


        public:
            inline TPtr * get_ptr(
                void)
                const noexcept
            {
                return ptr_.load();
            }

            template<
                class TPtrCompatible>
            inline bool set_ptr(
                TPtrCompatible * p_ptr)
                noexcept
            {
                assert(p_ptr);
                assert(p_ptr != ptr_.load());
                return release_ptr_cas(p_ptr);
            }

            inline bool reset_ptr(
                void)
                noexcept
            {
                return release_ptr_cas(nullptr);
            }

            template <
                class TPtrCompatible>
            inline bool release(
                TPtrCompatible ** p_out)
                noexcept
            {
                assert(*p_out != get_ptr());
                *p_out = ptr_.load();
                return ptr_.compare_exchange_strong(
                    *p_out,
                    nullptr);
            }

            template<
                class TPtrCompatible>
            inline bool exchange(
                TPtr ** p_out,
                TPtrCompatible * p_ptr)
                noexcept
            {
                assert(*p_out != get_ptr());
                assert(p_ptr);
                assert(p_ptr != get_ptr());
                *p_out = ptr_.load();
                return ptr_.compare_exchange_strong(
                    *p_out,
                    p_ptr);
            }

        }; // class body


        //////////////////////////////////////
        //              MEMBERS             //
        //////////////////////////////////////

    private:
        typedef typename sync_ptr<
            TPtr,
            TDeleter> sync_ptr_t;

        typedef typename body<
            TPtr,
            TDeleter> body_t;


    private:
        body_t *		body_;


        //////////////////////////////////////
        //              METHODS             //
        //////////////////////////////////////

    public:
        sync_ptr(
            void)
            noexcept
            // Members.
            : body_(new body_t())
        {}

        template<
            class TPtrCompatible>
        sync_ptr(
            TPtrCompatible * p_ptr)
            noexcept
            // Members.
            : body_(new body_t(p_ptr))
        {}

        sync_ptr(
            sync_ptr_t && p_other)
            noexcept
            // Members.
            : body_(p_other.body_)
        {
            p_other.body_ = nullptr;
        }

        sync_ptr(
            sync_ptr_t const & p_other)
            noexcept
            // Members.
            : body_(p_other.body_)
        {
            body_->ref();
            body_->ref_ptr();
        }

        ~sync_ptr(
            void)
            noexcept
        {
            if (body_)
            {
                body_->unref_ptr();
                body_->unref();
            }
        }

        inline sync_ptr_t & operator=(
            sync_ptr_t && p_other)
            noexcept
        {
            auto * tmp = p_other.body_;
            if (tmp != body_)
            {
                body_ = tmp;
                p_other.body_ = nullptr;
            }
            return *this;
        }

        inline sync_ptr_t & operator=(
            sync_ptr_t const & p_other)
            & noexcept
        {
            auto * tmp = p_other.body_;
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

        void swap(
            sync_ptr_t & p_rhs)
            noexcept
        {
            auto tmp = body_;
            body_ = p_rhs.body_;
            p_rhs.body_ = tmp;
        }


    public:
        inline size_t count(
            void)
            const noexcept
        {
            return body_->get_ref_count_ptr();
        }


    public:
        /**
        * \brief Set underlying pointer.
        * Free previous pointer on success.
        * Return true on success false otherwise.
        */
        template <
            class TPtrCompatible>
        inline bool reset(
            TPtrCompatible * p_ptr)
            noexcept
        {
            assert(p_ptr);
            return body_->set_ptr(p_ptr);
        }
        /**
        * \brief Set underlying pointer to null.
        * Free previous pointer on success.
        * Return true on success false otherwise.
        */
        inline bool reset(
            void)
            noexcept
        {
            return body_->reset_ptr();
        }


    public:
        /**
        * \brief Releases the ownership of the managed object if any.
        * Return the previously owned pointer and set the current to null.
        * Return true on success, false otherwise.
        */
        inline bool release(
            TPtr ** p_out)
            noexcept
        {
            return body_->release(p_out);
        }
        /**
        * \brief Set managed object and return previous one.
        * Return true on success, false otherwise.
        * On failure current object is unchanged and returned one is undefined.
        */
        template <
            class TPtrCompatible>
        inline bool exchange(
            TPtr ** p_out,
            TPtrCompatible * p_ptr)
            noexcept
        {
            return body_->exchange(p_out, p_ptr);
        }


    public:
        inline TPtr * get(
            void)
            const noexcept
        {
            return body_->get_ptr();
        }


    public:
        inline TPtr & operator*(
            void)
            const noexcept
        {
            return *get();
        }

        inline TPtr * operator->(
            void)
            const noexcept
        {
            return get();
        }


    public:
        inline bool valid(
            void)
            const noexcept
        {
            return (get() != nullptr);
        }

        inline operator bool(
            void)
            const noexcept
        {
            return valid();
        }

    }; // class sync_ptr


    ///////////////////////////////////////////////////////////////////////////////////////////
    //		MAKE
    ///////////////////////////////////////////////////////////////////////////////////////////

    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value, 
        cc::sync_ptr<TPtr, TDeleter>>::type
        make_sync(
            TArgs&&... p_args)
    {
        typedef typename sync_ptr<
            TPtr,
            TDeleter> sync_ptr_t;
        return (sync_ptr_t(new TPtr(std::forward<TArgs>(p_args)...)));
    }

    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter,
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
        cc::sync_ptr<TPtr, TDeleter>>::type
        make_sync_with_allocator(
            TAllocator<TPtr> const & p_allocator, 
            TArgs&&... p_args)
    {
        typedef typename sync_ptr<
            TPtr,
            TDeleter> sync_ptr_t;
        return (sync_ptr_t(p_allocator.allocate(std::forward<TArgs>(p_args)...)));
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

} // namespace cc


namespace std
{
    
    template <
        class TPtr,
        template <class T> class TDeleter>
    inline void swap(
        cc::sync_ptr<TPtr, TDeleter> & p_lhs,
        cc::sync_ptr<TPtr, TDeleter> & p_rhs)
        noexcept(noexcept(p_lhs.swap(p_rhs)))
    {
        p_lhs.swap(p_rhs);
    }

} // namespace std


///////////////////////////////////////////////////////////////////////////////////////////////////
//      RELATIONAL OPERATORS
///////////////////////////////////////////////////////////////////////////////////////////////////

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator==(
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
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
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
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
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    typedef typename cc::sync_ptr<TPtr1, TDeleter1>::pointer ptr1_t;
    typedef typename cc::sync_ptr<TPtr2, TDeleter2>::pointer ptr2_t;
    typedef typename std::common_type<ptr1_t, ptr2_t>::type common_t;
    return (std::less<common_t>()(p_lhs.get(), p_rhs.get()));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator>=(
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator>(
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    class TPtr2,
    template <class T> class TDeleter2>
inline bool operator<=(
    cc::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    cc::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}



template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator==(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t) 
    noexcept
{	
    return (!p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator==(
    std::nullptr_t,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
    noexcept
{	
    return (!p_rhs);
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator!=(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs) 
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator!=(
    std::nullptr_t p_lhs,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator<(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    typedef typename cc::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs.get(), p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator<(
    std::nullptr_t p_lhs,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    typedef typename cc::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs, p_rhs.get()));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator>=(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator>=(
    std::nullptr_t p_lhs,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator>(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator>(
    std::nullptr_t p_lhs,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator<=(
    cc::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

template<
    class TPtr,
    template <class T> class TDeleter >
inline bool operator<=(
    std::nullptr_t p_lhs,
    cc::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

#endif // __CC_SYNC_PTR_H__
