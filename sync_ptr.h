
#pragma once
#ifndef __EVE_MEMORY_SYNC_PTR_H__
#define __EVE_MEMORY_SYNC_PTR_H__

#include <atomic>
#include <functional>
#include <memory>


namespace eve
{
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
            TType * operator()(
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

            void operator()(
                TType * p_ptr) 
                const noexcept
            {	
                static_assert(
                    0 < sizeof(TType),
                    "can't delete an incomplete type");
                delete p_ptr;
            }

        }; // struct default_deleter

    } // namespace mem

} // namespace eve


namespace eve
{
    namespace mem
    {

        template<class TPtr>
        using sync_ptr_default_allocator	= eve::mem::default_allocator<TPtr>;

        template<class TPtr>
        using sync_ptr_default_deleter		= eve::mem::default_deleter<TPtr>;


        /** 
        * \class eve::mem::sync_ptr
        *
        * \brief Reference counted synchronized pointer.
        * Used to avoid cross module cycle.
        * When the original sync_ptr or one of its copy underlying raw pointer changes, 
        * all sync_ptr and copies point to the updated raw pointer.
        *
        * Call assignment operator or copy constructor to link objects to the same underlying raw pointer.
        *	eve::mem::sync_ptr<Foo> ptr1(new Foo());
        *	eve::mem::sync_ptr<Foo> ptr2(ptr1);
        *	eve::mem::sync_ptr<Foo> ptr3 = ptr1;
        *
        * Call reset(ptr) to update raw pointer on all linked instances.
        *	eve::mem::sync_ptr<Foo> ptr1(new Foo());
        *	eve::mem::sync_ptr<Foo> ptr2(ptr1);
        *	ptr1.reset(new Foo());
        */
        template <
            class TPtr,
            class TDeleter = sync_ptr_default_deleter<TPtr> >
        class sync_ptr final
        {

        public:
            typedef typename TPtr		pointer_type;
            typedef typename TDeleter	deleter_type;


        private:
            /**
            * \class eve::mem::ref_count_ptr
            *
            * \brief Reference counted template type pointer.
            * Deletes pointer when reference count is zero.
            * Reference count is atomic.
            */
            template<
                class TPtr,
                class TDeleter = sync_ptr_default_deleter<TPtr>>
            class ref_count_ptr final
            {

                //////////////////////////////////////
                //              MEMBERS             //
                //////////////////////////////////////

            private:
                std::atomic<size_t>	        ref_count_;
                std::atomic<size_t>	        ref_count_ptr_;
                std::atomic<TPtr *>		    ptr_;
                TDeleter				    deleter_;


                //////////////////////////////////////
                //              METHODS             //
                //////////////////////////////////////

            public:
                ref_count_ptr(ref_count_ptr<TPtr, TDeleter> const & p_other) = delete;
                ref_count_ptr(ref_count_ptr<TPtr, TDeleter> && p_other) = delete;
                void operator=(ref_count_ptr<TPtr, TDeleter> & p_arg) = delete;
                void operator=(ref_count_ptr<TPtr, TDeleter> && p_arg) = delete;

            public:
                /** 
                * \brief Construct default empty object. 
                */
                ref_count_ptr(
                    void)
                    noexcept
                    // Members.
                    : ref_count_(0)
                    , ref_count_ptr_(0)
                    , ptr_(nullptr)
                    , deleter_()
                {}

                /** 
                * \brief Construct with compatible deleter. 
                */
                ref_count_ptr(
                    TDeleter && p_deleter)
                    noexcept
                    // Members.
                    : ref_count_(0)
                    , ref_count_ptr_(0)
                    , ptr_(nullptr)
                    , deleter_(std::forward<TDeleter>(p_deleter))
                {}

                /** 
                * \brief Construct with compatible pointer. 
                */
                template<
                    class TPtrCompatible>
                ref_count_ptr(
                    TPtrCompatible * p_ptr)
                    noexcept 
                    // Members.
                    : ref_count_(1U)
                    , ref_count_ptr_(1U)
                    , ptr_(p_ptr)
                    , deleter_()
                {
                    assert(p_ptr);
                }

                /** 
                * \brief Construct with compatible pointer and compatible deleter. 
                */
                template<
                    class TPtrCompatible>
                ref_count_ptr(
                    TPtrCompatible * p_ptr,
                    TDeleter && p_deleter)
                    noexcept 
                    // Members.
                    : ref_count_(1U)
                    , ref_count_ptr_(1U)
                    , ptr_(p_ptr)
                    , deleter_(std::forward<TDeleter>(p_deleter))
                {
                    assert(p_ptr);
                }

            private:
                ~ref_count_ptr(
                    void) 
                    noexcept
                {}


            private:
                /** 
                * \brief Delete this. 
                * Called when this reference count drops to zero. 
                */
                inline void release(
                    void) 
                    noexcept
                {
                    delete this;
                }
                /** 
                * \brief Delete contained pointer.
                * Called when pointer reference count drops to zero.
                */
                inline void release_ptr(
                    void) 
                    noexcept
                {
                    auto ptr = ptr_.load();
                    ptr_.store(nullptr);
                    if (ptr)
                    {
                        deleter_(ptr_);
                    }
                }


            public:
                /** 
                * \brief Increments this reference count. 
                */
                inline void ref(
                    void) 
                    noexcept
                {
                    ref_count_.fetch_add(1U, std::memory_order_acquire);
                }
                /** 
                * \brief Decrements this reference count, 
                * release this if reference count drops to zero. 
                */
                inline void unref(
                    void) 
                    noexcept
                {
                    if (ref_count_.fetch_sub(1U, std::memory_order_release) == 1U)
                    {
                        release();
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
                        ref_count_ptr_.fetch_add(1U, std::memory_order_acquire);
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
                        if (ref_count_ptr_.fetch_sub(1U, std::memory_order_release) == 1U)
                        {
                            release_ptr();
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
                    return ref_count_.load(std::memory_order_relaxed);
                }
                inline size_t get_ref_count_ptr(
                    void) 
                    const noexcept
                {
                    return ref_count_ptr_.load(std::memory_order_relaxed);
                }


            public:
                inline TPtr * get_ptr(
                    void) 
                    const noexcept
                {
                    return ptr_.load();
                }


            public:
                inline void set_ptr(
                    TPtr * p_ptr) 
                    noexcept
                {
                    assert(p_ptr);

                    if (ptr_.load() != p_ptr)
                    {
                        release_ptr();
                        ptr_.store(p_ptr);
                    }
                }

            public:
                inline void reset(
                    void) 
                    noexcept
                {
                    release_ptr();
                }

            }; // class ref_count_ptr


            //////////////////////////////////////
            //              MEMBERS             //
            //////////////////////////////////////

        private:
            ref_count_ptr<
                TPtr, 
                TDeleter> *		ref_;


            //////////////////////////////////////
            //              METHODS             //
            //////////////////////////////////////

        public:
            /**
            * \brief Construct default empty object.
            */
            sync_ptr(
                void)
                noexcept
                // Members.
                : ref_(new ref_count_ptr<TPtr, TDeleter>())
            {}

            /**
            * \brief Construct with deleter.
            */
            sync_ptr(
                TDeleter && p_deleter)
                noexcept
                // Members.
                : ref_(new ref_count_ptr<TPtr, TDeleter>(std::forward<TDeleter>(p_deleter)))
            {}

            /**
            * \brief Construct with compatible pointer.
            */
            template<
                class TPtrCompatible>
            sync_ptr(
                TPtrCompatible * p_ptr)
                noexcept
                // Members.
                : ref_(new ref_count_ptr<TPtr, TDeleter>(p_ptr))
            {}

            /**
            * \brief Construct with compatible pointer and deleter.
            */
            template<
                class TPtrCompatible>
            sync_ptr(
                TPtrCompatible * p_ptr,
                TDeleter && p_deleter)
                noexcept
                // Members.
                : ref_(new ref_count_ptr<TPtr, TDeleter>(p_ptr, std::forward<TDeleter>(p_deleter)))
            {}

            sync_ptr(
                sync_ptr<TPtr, TDeleter> && p_other)
                noexcept
                // Members.
                : ref_(p_other.ref_)
            {
                p_other.ref_ = nullptr;
            }

            sync_ptr(
                sync_ptr<TPtr, TDeleter> const & p_other)
                noexcept
                // Members.
                : ref_(p_other.ref_)
            {
                ref_->ref();
                ref_->ref_ptr();
            }

            ~sync_ptr(
                void) 
                noexcept
            {
                if (ref_)
                {
                    ref_->unref_ptr();
                    ref_->unref();
                }
            }

            inline eve::mem::sync_ptr<TPtr, TDeleter> & operator=(
                sync_ptr<TPtr, TDeleter> && p_other)
                noexcept
            {
                auto * tmp = p_other.ref_;
                if (tmp != ref_)
                {
                    ref_ = tmp;
                    p_other.ref_ = nullptr;
                }
                return *this;
            }

            inline eve::mem::sync_ptr<TPtr, TDeleter> & operator=(
                sync_ptr<TPtr, TDeleter> const & p_other)
                & noexcept
            {
                auto * tmp = p_other.ref_;
                if (tmp != ref_)
                {
                    ref_->unref_ptr();
                    ref_->unref();

                    ref_ = tmp;
                    ref_->ref();
                    ref_->ref_ptr();
                }
                return *this;
            }

            void swap(
                sync_ptr<TPtr, TDeleter> & p_rhs)
                noexcept
            {
                auto tmp = ref_;
                ref_ = p_rhs.ref_;
                p_rhs.ref_ = tmp;
            }


        public:
            inline size_t count(
                void)
                const noexcept
            {
                return ref_->get_ref_count_ptr();
            }


        public:
            inline void reset(
                TPtr * p_ptr) 
                noexcept
            {
                ref_->set_ptr(p_ptr);
            }

            inline void reset(
                void) 
                noexcept
            {
                ref_->reset();
            }


        public:
            inline TPtr * get(
                void) 
                const noexcept
            {
                return ref_->get_ptr();
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
        //		DEFAULT
        ///////////////////////////////////////////////////////////////////////////////////////////

        template<
            class TPtr, 
            class... TArgs> 
        inline typename std::enable_if<!std::is_array<TPtr>::value, eve::mem::sync_ptr<TPtr>>::type make_sync(
            TArgs&&... p_args)
        {
            return (eve::mem::sync_ptr<TPtr>(
                        new TPtr(std::forward<TArgs>(p_args)...)));
        }

        template<
            class TPtr, 
            class... TArgs>
        typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync(
            TArgs&&...) 
            = delete;



        ///////////////////////////////////////////////////////////////////////////////////////////
        //		DELETER
        ///////////////////////////////////////////////////////////////////////////////////////////

        template<
            class TPtr, 
            class TDeleter,
            class... TArgs>
        inline typename std::enable_if<!std::is_array<TPtr>::value, eve::mem::sync_ptr<TPtr, TDeleter>>::type make_sync_with_deleter(
            TDeleter && p_deleter,
            TArgs&&... p_args)
        {
            return (eve::mem::sync_ptr<TPtr, TDeleter>(
                        new TPtr(std::forward<TArgs>(p_args)...), 
                        std::forward<TDeleter>(p_deleter)));
        }

        template<
            class TPtr, 
            class TDeleter, 
            class... TArgs>
        typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync_with_deleter(
            TDeleter && p_deleter,
            TArgs&&...)
            = delete;
        


        ///////////////////////////////////////////////////////////////////////////////////////////
        //		ALLOCATOR
        ///////////////////////////////////////////////////////////////////////////////////////////

        template<
            class TPtr, 
            class TAllocator, 
            class... TArgs>
        inline typename std::enable_if<!std::is_array<TPtr>::value, eve::mem::sync_ptr<TPtr>>::type make_sync_with_allocator(
            TAllocator const & p_allocator,
            TArgs&&... p_args)
        {
            return (eve::mem::sync_ptr<TPtr>(
                        p_allocator(std::forward<TArgs>(p_args)...)));
        }

        template<
            class TPtr, 
            class TAllocator, 
            class... TArgs>
        typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync_with_allocator(
            TAllocator const & p_allocator,
            TArgs&&...)
            = delete;



        ///////////////////////////////////////////////////////////////////////////////////////////
        //		ALLOCATOR AND DELETER
        ///////////////////////////////////////////////////////////////////////////////////////////

        template<
            class TPtr, 
            class TAllocator, 
            class TDeleter, 
            class... TArgs>
        inline typename std::enable_if<!std::is_array<TPtr>::value, eve::mem::sync_ptr<TPtr, TDeleter>>::type make_sync_with_allocator_and_deleter(
            TAllocator const & p_allocator,
            TDeleter && p_deleter,
            TArgs&&... p_args)
        {
            return (eve::mem::sync_ptr<TPtr, TDeleter>(
                        p_allocator(std::forward<TArgs>(p_args)...),
                        std::forward<TDeleter>(p_deleter)));
        }

        template<
            class TPtr, 
            class TAllocator, 
            class TDeleter, 
            class... TArgs>
        typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync_with_allocator(
            TAllocator const & p_allocator,
            TDeleter && p_deleter,
            TArgs&&...)
            = delete;

    } // namespace mem

} // namespace eve


namespace std
{
    
    template <
        class TPtr,
        class TDeleter>
    inline void swap(
        eve::mem::sync_ptr<TPtr, TDeleter> & p_lhs,
        eve::mem::sync_ptr<TPtr, TDeleter> & p_rhs)
        noexcept(noexcept(p_lhs.swap(p_rhs)))
    {
        p_lhs.swap(p_rhs);
    }

} // namespace std


///////////////////////////////////////////////////////////////////////////////////////////////////
//      OPERATORS
///////////////////////////////////////////////////////////////////////////////////////////////////

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator==(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
    noexcept
{	
    return (p_lhs.get() == p_rhs.get());
}

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator!=(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
    noexcept
{
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator<(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    typedef typename eve::mem::sync_ptr<TPtr1, TDeleter1>::pointer ptr1_t;
    typedef typename eve::mem::sync_ptr<TPtr2, TDeleter2>::pointer ptr2_t;
    typedef typename std::common_type<ptr1_t, ptr2_t>::type common_t;
    return (std::less<common_t>()(p_lhs.get(), p_rhs.get()));

}

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator>=(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator>(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr1,
    class TDeleter1,
    class TPtr2,
    class TDeleter2>
inline bool operator<=(
    eve::mem::sync_ptr<TPtr1, TDeleter1> const & p_lhs,
    eve::mem::sync_ptr<TPtr2, TDeleter2> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}



template<
    class TPtr,
    class TDeleter>
inline bool operator==(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t) 
    noexcept
{	
    return (!p_lhs);
}

template<
    class TPtr,
    class TDeleter>
inline bool operator==(
    std::nullptr_t,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs) 
    noexcept
{	
    return (!p_rhs);
}

template<
    class TPtr,
    class TDeleter>
inline bool operator!=(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs) 
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator!=(
    std::nullptr_t p_lhs,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs) 
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator<(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    typedef typename eve::mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs.get(), p_rhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator<(
    std::nullptr_t p_lhs,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    typedef typename eve::mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs, p_rhs.get()));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator>=(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator>=(
    std::nullptr_t p_lhs,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator>(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    class TDeleter>
inline bool operator>(
    std::nullptr_t p_lhs,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    class TDeleter>
inline bool operator<=(
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

template<
    class TPtr,
    class TDeleter>
inline bool operator<=(
    std::nullptr_t p_lhs,
    eve::mem::sync_ptr<TPtr, TDeleter> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

#endif // __EVE_MEMORY_SYNC_PTR_H__
