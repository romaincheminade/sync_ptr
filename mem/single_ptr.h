
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
        static std::atomic<TPtr *>      atomic_ptr_;
        static std::atomic<bool>        destroyed_;


    public:
        single_ptr(void) noexcept = default;
        ~single_ptr(void) noexcept = default;


    public:
        single_ptr(single_ptr<TPtr, TDeleter> && p_rhs) noexcept = delete;
        single_ptr(single_ptr<TPtr, TDeleter> const & p_rhs) noexcept = delete;
        single_ptr<TPtr, TDeleter> & operator=(single_ptr<TPtr, TDeleter> && p_rhs) noexcept = delete;
        single_ptr<TPtr, TDeleter> & operator=(single_ptr<TPtr, TDeleter> const & p_rhs) & noexcept = delete;


    public:
        static TPtr * get(void) const noexcept
        {
            auto res = atomic_ptr_.load(std::memory_order_acquire);
            if (!res)
            {
                if (destroyed_)
                {
                    destroyed_ = false;
                }
//                 pInstance_ = CreationPolicy<T>::Create();
//                 LifetimePolicy<T>::ScheduleDestruction(pInstance_,
//                     &DestroySingleton);

                atomic_ptr_.store(res, std::memory_order_release);
                destroyed_ = false;
            }

            return res;
        }

        static TPtr * operator->(void) const noexcept
        {
            return get();
        }

        static TPtr & operator*(void) const noexcept
        {
            return *get();
        }

    }; // class single_ptr

    template <class TPtr, template <class T> class TDeleter>
    std::atomic<TPtr *> single_ptr<TPtr, TDeleter>::atomic_ptr_ = nullptr;

    template <class TPtr, template <class T> class TDeleter>
    bool single_ptr<TPtr, TDeleter>::destroyed_ = false;

} // namespace mem

//=============================================================================

#endif // __MEM_SINGLE_PTR_H__
