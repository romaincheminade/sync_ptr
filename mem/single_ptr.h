
#ifndef __MEM_SINGLE_PTR_H__
#define __MEM_SINGLE_PTR_H__


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
    class single_ptr final
        : private TDeleter<TPtr>
    {

    private:
        TPtr *     ptr_;


    public:
        single_ptr(void) noexcept = default;
        ~single_ptr(void) noexcept = default;


    private:
        using single_ptr_t = single_ptr<TPtr, TDeleter>;
    public:
        single_ptr(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr(single_ptr_t const & p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t const & p_rhs) & noexcept = delete;


    public:
        TPtr * get(void) const noexcept
        {
            return ptr_;
        }

        TPtr * operator->(void) const noexcept
        {
            return get();
        }

        TPtr & operator*(void) const noexcept
        {
            return *get();
        }

    }; // class single_ptr

} // namespace mem

//=============================================================================

#endif // __MEM_SINGLE_PTR_H__
