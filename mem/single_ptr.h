
#ifndef __MEM_SINGLE_PTR_H__
#define __MEM_SINGLE_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif


namespace mem
{

    template <
        class TPtr>
    class single_ptr final
    {

    private:
        using single_ptr_t = single_ptr<TPtr>;

    private:
        static TPtr *                   ptr_;
        static std::atomic<TPtr *>      atomic_ptr_;
        static bool                     destroyed_;
        static std::mutex               mutex_;


    private:
        single_ptr(void) noexcept = default;
        ~single_ptr(void) noexcept = default;


    public:
        single_ptr(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr(single_ptr_t const & p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t const & p_rhs) & noexcept = delete;


    private:
        static void create(void)
        {
            static TPtr instance;
            ptr_ = &instance;
        }

        static void recreate(void)
        {
            create();
            new(ptr_) TPtr;
        }

        static void store(void)
        {
            std::atexit(reclame);
            atomic_ptr_.store(ptr_, std::memory_order_release);
            destroyed_ = false;
        }

        static void reclame(void)
        {
            atomic_ptr_.store(nullptr, std::memory_order_release);

            std::lock_guard<std::mutex> l(mutex_);
            ptr_->~TPtr();
            ptr_ = nullptr;
            destroyed_ = true;
        }


    public:
        static TPtr * instance(void)
        {
            if(!atomic_ptr_.load(std::memory_order_acquire))
            {
                std::unique_lock<std::mutex> l(mutex_);
                if (!ptr_)
                {
                    if (destroyed_)
                    {
                        recreate();
                    }
                    else
                    {
                        create();
                    }
                    store();
                }
                l.unlock();
            }
            return ptr_;
        }

    }; // class single_ptr

} // namespace mem

//=============================================================================

template <class TPtr>
TPtr * mem::single_ptr<TPtr>::ptr_ = nullptr;

template <class TPtr>
std::atomic<TPtr *> mem::single_ptr<TPtr>::atomic_ptr_;

template <class TPtr>
bool mem::single_ptr<TPtr>::destroyed_ = false;

template <class TPtr>
std::mutex mem::single_ptr<TPtr>::mutex_;

#endif // __MEM_SINGLE_PTR_H__
