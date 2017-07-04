
#ifndef __MEM_ALLOCATION_POLICY_H__
#define __MEM_ALLOCATION_POLICY_H__

#include <cassert>
#include <utility>


namespace mem
{

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

        void deallocate(
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

        void deallocate(
            TType * p_ptr)
            const noexcept
        {}

    }; // struct noop_deleter

} // namespace mem

#endif // __MEM_ALLOCATION_POLICY_H__
