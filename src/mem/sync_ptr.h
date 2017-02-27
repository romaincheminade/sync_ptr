
#ifndef __MEMORY_SYNC_PTR_H__
#define __MEMORY_SYNC_PTR_H__

#include <cassert>

#ifndef __EVE_MEMORY_SYNC_PTR_POLICY_H__
#include "mem/sync_ptr_policy.h"
#endif


namespace mem
{

    template<class TPtr>
    using sync_ptr_allocator			= default_allocator<TPtr>;

    template<class TPtr>
    using sync_ptr_deleter				= default_deleter<TPtr>;

    template<class TPtr>
    using sync_ptr_holder				= ptr_holder_ts<TPtr>;

    using sync_ptr_ref_counter			= atomic_ref_counter;


    /** 
    * \class mem::sync_ptr
    *
    * \brief Reference counted synchronized pointer.
    * Used to avoid cross module cycle.
    * When the original sync_ptr or one of its copy underlying raw pointer changes, 
    * all sync_ptr and copies point to the updated raw pointer.
    */
    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter,
        template <class T> class THolder = sync_ptr_holder,
        class TRefCounter = sync_ptr_ref_counter>
    class sync_ptr final
    {

    public:
        typedef typename TPtr		    pointer_type;
        typedef typename TDeleter<TPtr>	deleter_type;
        typedef typename THolder<TPtr>	holder_type;
        typedef typename TRefCounter	reference_counter_type;


    private:
        /**
        * \class mem::sync_ptr::ref_count_ptr
        *
        * \brief Reference counted template type pointer.
        * Deletes pointer when reference count is zero.
        * Reference count is atomic.
        */
        template<
            class TPtr,
            template <class T> class TDeleter,
            template <class T> class THolder,
            class TRefCounter>
        class ref_count_ptr final
            : private TDeleter<TPtr>
            , private THolder<TPtr>
            , private TRefCounter
        {

            //////////////////////////////////////
            //              METHODS             //
            //////////////////////////////////////

        public:
            ref_count_ptr(ref_count_ptr const & p_other) = delete;
            ref_count_ptr(ref_count_ptr && p_other) = delete;
            void operator=(ref_count_ptr & p_arg) = delete;
            void operator=(ref_count_ptr && p_arg) = delete;

        public:
            /** 
            * \brief Construct default empty object. 
            */
            ref_count_ptr(
                void)
                noexcept
            {}
            
            /** 
            * \brief Construct with compatible pointer. 
            */
            template<
                class TPtrCompatible>
            ref_count_ptr(
                TPtrCompatible * p_ptr)
                noexcept 
                // Inheritance.
                : THolder<TPtr>(p_ptr)
            {
                assert(p_ptr);
                increment_ptr();
            }

        private:
            ~ref_count_ptr(
                void) 
                noexcept
            {}


        private:
            inline void release_this(
                void) 
                noexcept
            {
                delete this;
            }

            inline void release_ptr(
                TPtr * p_ptr)
                noexcept
            {
                static_assert(
                    noexcept(free(p_ptr)),
                    "Deleter policy must offer no-throw guarantee.");

                static_assert(
                    noexcept(set(p_ptr)),
                    "Pointer holder policy must offer no-throw guarantee.");

                auto p = set(p_ptr);                    
                if (p)
                {
                    free(p);
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
                static_assert(
                    noexcept(increment()),
                    "Reference counter policy must offer no-throw guarantee.");

                increment();
            }
            /** 
            * \brief Decrements this reference count. 
            * Release this if reference count drops to zero. 
            */
            inline void unref(
                void) 
                noexcept
            {
                static_assert(
                    noexcept(decrement()),
                    "Reference counter policy must offer no-throw guarantee.");

                if (decrement() == 1U)
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
                static_assert(
                    noexcept(increment_ptr()),
                    "Reference counter policy must offer no-throw guarantee.");

                if (get_ptr())
                {
                    increment_ptr();
                }
            }
            /** 
            * \brief Decrements pointer reference count. 
            * Release pointer if reference count drops to zero. 
            */
            inline void unref_ptr(
                void) 
                noexcept
            {
                static_assert(
                    noexcept(decrement_ptr()),
                    "Reference counter policy must offer no-throw guarantee.");

                if (get_ptr())
                {
                    if (decrement_ptr() == 1U)
                    {
                        release_ptr(nullptr);
                    }
                }
            }


        public:
            template<
                class TPtrCompatible>
            inline void reset_ptr(
                TPtrCompatible * p_ptr)
                noexcept
            {
                assert(p_ptr);
                assert(p_ptr != get_ptr());
                release_ptr(p_ptr);
            }

            inline void reset_ptr(
                void)
                noexcept
            {
                release_ptr(nullptr);
            }


        public:
            inline TPtr * release(
                void)
                noexcept
            {
                static_assert(
                    noexcept(set(nullptr)),
                    "Pointer holder policy must offer no-throw guarantee.");

                return set(nullptr);
            }

            template<
                class TPtrCompatible>
            inline TPtr * exchange(
                TPtrCompatible * p_ptr)
                noexcept
            {
                static_assert(
                    noexcept(set(p_ptr)),
                    "Pointer holder policy must offer no-throw guarantee.");

                assert(p_ptr);
                assert(p_ptr != get_ptr());
                return set(p_ptr);
            }


            ///////////////////////////////////////////////////////////////////////////////////////
            //		GET / SET
            ///////////////////////////////////////////////////////////////////////////////////////

        public:
            inline size_t get_ref_count(
                void)
                const noexcept
            {
                static_assert(
                    noexcept(count()),
                    "Reference counter policy must offer no-throw guarantee.");

                return count();
            }
            
            inline size_t get_ref_count_ptr(
                void)
                const noexcept
            {
                static_assert(
                    noexcept(count_ptr()),
                    "Reference counter policy must offer no-throw guarantee.");

                return count_ptr();
            }

            inline TPtr * get_ptr(
                void)
                const noexcept
            {
                static_assert(
                    noexcept(get()),
                    "Pointer holder policy must offer no-throw guarantee.");

                return get();
            }

        }; // class ref_count_ptr


        //////////////////////////////////////
        //              MEMBERS             //
        //////////////////////////////////////

    private:
        typedef typename sync_ptr<
            TPtr,
            TDeleter,
            THolder,
            TRefCounter> sync_ptr_t;

        typedef typename ref_count_ptr<
            TPtr,
            TDeleter,
            THolder,
            TRefCounter> ref_ptr_t;

    private:
        ref_ptr_t *		ref_;


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
            : ref_(new ref_ptr_t())
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
            : ref_(new ref_ptr_t(p_ptr))
        {}

        sync_ptr(
            sync_ptr && p_other)
            noexcept
            // Members.
            : ref_(p_other.ref_)
        {
            p_other.ref_ = nullptr;
        }

        sync_ptr(
            sync_ptr const & p_other)
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

        inline sync_ptr_t & operator=(
            sync_ptr_t && p_other)
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

        inline sync_ptr_t & operator=(
            sync_ptr_t const & p_other)
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
            sync_ptr & p_rhs)
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
        /** 
        * \brief Set underlying pointer.
        * Free previous pointer.
        */
        inline void reset(
            TPtr * p_ptr) 
            noexcept
        {
            assert(p_ptr);
            ref_->reset_ptr(p_ptr);
        }
        /**
        * \brief Set underlying pointer to null.
        * Free previous pointer.
        */
        inline void reset(
            void)
            noexcept
        {
            ref_->reset_ptr();
        }


    public:
        /**
        * \brief Releases the ownership of the managed object if any.
        * Return the previously owned pointer and set the current to null.
        */
        inline TPtr * release(
            void)
            noexcept
        {
            return ref_->release();
        }
        /**
        * \brief Set managed object and return previous one.
        */
        inline TPtr * exchange(
            TPtr * p_ptr)
            noexcept
        {
            return ref_->exchange(p_ptr);
        }


    public:
        inline TPtr * get(
            void) 
            const noexcept
        {
            return ref_->get_ptr();
        }

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
        template <class T> class THolder = sync_ptr_holder,
        class TRefCounter = sync_ptr_ref_counter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value, 
        mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter>>::type
        make_sync(
            TArgs&&... p_args)
    {
        typedef typename sync_ptr<
            TPtr,
            TDeleter,
            THolder,
            TRefCounter> sync_ptr_t;
        return (sync_ptr_t(new TPtr(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TDeleter,
        template <class T> class THolder,
        class TRefCounter,
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
        template <class T> class THolder = sync_ptr_holder,
        class TRefCounter = sync_ptr_ref_counter,
        class... TArgs>
    inline typename std::enable_if<
        !std::is_array<TPtr>::value, 
        mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter>>::type
        make_sync_with_allocator(
            TAllocator<TPtr> const & p_allocator, 
            TArgs&&... p_args)
    {
        typedef typename sync_ptr<
            TPtr,
            TDeleter,
            THolder,
            TRefCounter> sync_ptr_t;
        return (sync_ptr_t(p_allocator.allocate(std::forward<TArgs>(p_args)...)));
    }

    template<
        class TPtr,
        template <class T> class TAllocator,
        template <class T> class TDeleter,
        template <class T> class THolder,
        class TRefCounter,
        class... TArgs>
    typename std::enable_if<std::extent<TPtr>::value != 0, void>::type make_sync_with_allocator(
        TAllocator<TPtr> const & p_allocator, 
        TArgs&&...)
        = delete;

} // namespace mem


namespace std
{
    
    template <
        class TPtr,
        template <class T> class TDeleter,
        template <class T> class THolder,
        class TRefCounter>
    inline void swap(
        mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> & p_lhs,
        mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> & p_rhs)
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
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator==(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
    noexcept
{	
    return (p_lhs.get() == p_rhs.get());
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator!=(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
    noexcept
{
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator<(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
{	
    typedef typename mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1>::pointer ptr1_t;
    typedef typename mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2>::pointer ptr2_t;
    typedef typename std::common_type<ptr1_t, ptr2_t>::type common_t;
    return (std::less<common_t>()(p_lhs.get(), p_rhs.get()));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator>=(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator>(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr1,
    template <class T> class TDeleter1,
    template <class T> class THolder1,
    class TRefCounter1,
    class TPtr2,
    template <class T> class TDeleter2,
    template <class T> class THolder2,
    class TRefCounter2 >
inline bool operator<=(
    mem::sync_ptr<TPtr1, TDeleter1, THolder1, TRefCounter1> const & p_lhs,
    mem::sync_ptr<TPtr2, TDeleter2, THolder2, TRefCounter2> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}



template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator==(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t) 
    noexcept
{	
    return (!p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator==(
    std::nullptr_t,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
    noexcept
{	
    return (!p_rhs);
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator!=(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t p_rhs) 
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator!=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
    noexcept
{	
    return (!(p_lhs == p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator<(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    typedef typename mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs.get(), p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator<(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
{	
    typedef typename mem::sync_ptr<TPtr, TDeleter>::pointer _Ptr;
    return (std::less<_Ptr>()(p_lhs, p_rhs.get()));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator>=(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator>=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
{	
    return (!(p_lhs < p_rhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator>(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator>(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
{	
    return (p_rhs < p_lhs);
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator<=(
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_lhs,
    std::nullptr_t p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

template<
    class TPtr,
    template <class T> class TDeleter,
    template <class T> class THolder,
    class TRefCounter >
inline bool operator<=(
    std::nullptr_t p_lhs,
    mem::sync_ptr<TPtr, TDeleter, THolder, TRefCounter> const & p_rhs)
{	
    return (!(p_rhs < p_lhs));
}

#endif // __MEMORY_SYNC_PTR_H__
